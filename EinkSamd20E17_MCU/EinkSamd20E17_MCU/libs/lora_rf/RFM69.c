
//for writing data to RF FIFO
//>> sendFrame(0, &rf_tx_str, sizeof(rf_tx_str));



#include "hw_driver.h"
#include "RFM69registers.h"
#include "RFM69.h"


volatile uint8_t DATALEN;
volatile uint8_t SENDERID;
volatile uint8_t TARGETID;                 // should match _address
volatile uint8_t PAYLOADLEN;
volatile uint8_t ACK_REQUESTED;
volatile uint8_t ACK_RECEIVED;             // should be polled immediately after sending a packet with ACK request
volatile int16_t RSSI;                     // most accurate RSSI during reception (closest to the reception)
volatile uint8_t mode = RF69_MODE_STANDBY; // should be protected?
volatile uint8_t inISR = 0; 
uint8_t isRFM69HW = 1;                     // if RFM69HW model matches high power enable possible
uint8_t address;                           //nodeID
uint8_t powerLevel = 20;
uint8_t promiscuousMode = 0;
uint8_t rfDebugStr[128];

unsigned long millis_current;
rfHeader rfRxHeader;

// freqBand must be selected from 315, 433, 868, 915
uint8_t rfm69_init(uint16_t freqBand, uint8_t nodeID, uint8_t networkID)
{	
	RF_Reset();
	uint8_t tmp_data = readReg(REG_VERSION);
	//tmp_data = readReg(REG_OPMODE);
	
	writeReg(REG_OPMODE, RF_OPMODE_STANDBY);
	
	tmp_data = readReg(REG_IRQFLAGS1);
	
	writeReg(REG_FIFOTHRESH, 0x8f);
	writeReg(REG_TESTDAGC, 0x30);
	writeReg(REG_TESTPA1, 0x55); //only for HW
	writeReg(REG_TESTPA2, 0x70); //only for HW
	
	tmp_data = readReg(REG_SYNCCONFIG);
	writeReg(REG_SYNCVALUE1, 0x2d);
	writeReg(REG_SYNCVALUE2, 0xd4);
	writeReg(REG_SYNCCONFIG, 0x88);
	
	writeReg(REG_DATAMODUL, RF_DATAMODUL_MODULATIONSHAPING_01);
	writeReg(REG_BITRATEMSB, 0x00);
	writeReg(REG_BITRATELSB, 0x80);
	writeReg(REG_FDEVMSB, 0x10);
	writeReg(REG_FDEVLSB, 0x00);
	
	writeReg(REG_RXBW, RF_RXBW_DCCFREQ_111);
	writeReg(REG_AFCBW, RF_AFCBW_DCCFREQAFC_111);
	writeReg(REG_PACKETCONFIG1, 0xD0);
	
	writeReg(REG_PREAMBLEMSB, 0x00);
	writeReg(REG_PREAMBLELSB , 0x04);
	
	writeReg(REG_FRFMSB, RF_FRFMSB_868);
	writeReg(REG_FRFMID, RF_FRFMID_868);
	writeReg(REG_FRFLSB, RF_FRFLSB_868);
	
	tmp_data = readReg(REG_PACKETCONFIG2); //2
	writeReg(REG_PACKETCONFIG2, RF_PACKET2_AUTORXRESTART_ON);

	writeReg(REG_PALEVEL, RF_PALEVEL_OUTPUTPOWER_11111);
	writeReg(REG_DIOMAPPING1 , RF_DIOMAPPING1_DIO0_01);
	
	//tmp_data = readReg(REG_OPMODE);
	writeReg(REG_OPMODE, RF_OPMODE_RECEIVER);
	
	
    //encrypt(0);
    //setHighPower(isRFM69HW);        // called regardless if it's a RFM69W or RFM69HW
    //setMode(RF69_MODE_STANDBY);
    //while ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00);

    //address = nodeID;
    //setAddress(address);            // setting this node id
    //setNetwork(networkID);
	return tmp_data;
}

// set this node's address
void setAddress(uint8_t addr)
{
    writeReg(REG_NODEADRS, addr);
}

// set network address
void setNetwork(uint8_t networkID)
{
    writeReg(REG_SYNCVALUE2, networkID);
}

uint8_t canSend()
{
    if (mode == RF69_MODE_RX && PAYLOADLEN == 0 && readRSSI(0) < CSMA_LIMIT) // if signal stronger than -100dBm is detected assume channel activity
    {
        setMode(RF69_MODE_STANDBY);
        return 1;
    }
    return 0;
}

// Transmit data
//void send(uint8_t toAddress, const void* buffer, uint8_t bufferSize, uint8_t requestACK)
//{
    //writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
    ////millis_current = millis();
	//delay_ms(10);
  //
	//uint8_t loops = 5;
	//while (!canSend() || !loops ){ 
		//receiveDone();
		//delay_ms(1000);
		//loops--;
		//}//                         <<<<Need to optimize to freezing protection
    ////sendFrame(toAddress, buffer, bufferSize, requestACK, 0);
//}

// check whether an ACK was requested in the last received packet (non-broadcasted packet)
uint8_t ACKRequested() 
{
    return ACK_REQUESTED && (TARGETID != RF69_BROADCAST_ADDR);
}

// should be called immediately after reception in case sender wants ACK
void sendACK(const void* buffer, uint8_t bufferSize)
{
    ACK_REQUESTED = 0;   // TWS added to make sure we don't end up in a timing race and infinite loop sending Acks
    uint8_t sender = SENDERID;
    int16_t _RSSI = RSSI; // save payload received RSSI value
    writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
   
    while (!canSend()) receiveDone();
    SENDERID = sender;    // TWS: Restore SenderID after it gets wiped out by receiveDone() n.b. actually now there is no receiveDone() :D
    //sendFrame(sender, buffer, bufferSize, 0, 1);
    RSSI = _RSSI; // restore payload RSSI
}

// set *transmit/TX* output power: 0=min, 31=max
// this results in a "weaker" transmitted signal, and directly results in a lower RSSI at the receiver
// the power configurations are explained in the SX1231H datasheet (Table 10 on p21; RegPaLevel p66): http://www.semtech.com/images/datasheet/sx1231h.pdf
// valid powerLevel parameter values are 0-31 and result in a directly proportional effect on the output/transmission power
// this function implements 2 modes as follows:
//       - for RFM69W the range is from 0-31 [-18dBm to 13dBm] (PA0 only on RFIO pin)
//       - for RFM69HW the range is from 0-31 [5dBm to 20dBm]  (PA1 & PA2 on PA_BOOST pin & high Power PA settings - see section 3.3.7 in datasheet, p22)
void setPowerLevel(uint8_t powerLevel)
{
    uint8_t _powerLevel = powerLevel;
    if (isRFM69HW==1) _powerLevel /= 2;
    writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0xE0) | _powerLevel);
	readReg(REG_PALEVEL);
}

//put transceiver in sleep mode to save battery - to wake or resume receiving just call receiveDone()
void RF_sleep() 
{
    setMode(RF69_MODE_SLEEP);
}

uint8_t readTemperature(uint8_t calFactor) // returns centigrade
{
    setMode(RF69_MODE_STANDBY);
    writeReg(REG_TEMP1, RF_TEMP1_MEAS_START);
    while ((readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
    return ~readReg(REG_TEMP2) + COURSE_TEMP_COEF + calFactor; // 'complement' corrects the slope, rising temp = rising val
} // COURSE_TEMP_COEF puts reading in the ballpark, user can add additional correction

// return the frequency (in Hz)
uint32_t getFrequency()
{
    return RF69_FSTEP * (((uint32_t) readReg(REG_FRFMSB) << 16) + ((uint16_t) readReg(REG_FRFMID) << 8) + readReg(REG_FRFLSB));
}

// set the frequency (in Hz)
void setFrequency(uint32_t freqHz)
{
    uint8_t oldMode = mode;
    if (oldMode == RF69_MODE_TX) {
        setMode(RF69_MODE_RX);
    }
    freqHz /= RF69_FSTEP; // divide down by FSTEP to get FRF
    writeReg(REG_FRFMSB, freqHz >> 16);
    writeReg(REG_FRFMID, freqHz >> 8);
    writeReg(REG_FRFLSB, freqHz);
    if (oldMode == RF69_MODE_RX) {
        setMode(RF69_MODE_SYNTH);
    }
    setMode(oldMode);
}

// Read byte from register
uint8_t readReg(uint8_t addr)
{
    RF_select();
    SPI0_Write_byte(addr & 0x7F);
    uint8_t regval = SPI0_Read_byte();
    RF_unselect();
    return regval;
}

// Write byte to register
void writeReg(uint8_t addr, uint8_t value)
{
    RF_select();
    SPI0_Write_byte(addr | 0x80);
    SPI0_Write_byte(value);
    RF_unselect();
}

// To enable encryption: radio.encrypt("ABCDEFGHIJKLMNOP");
// To disable encryption: encrypt(null) or encrypt(0)
// KEY HAS TO BE 16 bytes !!!
void encrypt(const char* key) 
{
    setMode(RF69_MODE_STANDBY);
    
	RF_select();
	if (key != 0)
    {
        
        SPI0_Write_byte(REG_AESKEY1 | 0x80);
        for (uint8_t i = 0; i < 16; i++)
            //debug_var = key[i];
			SPI0_Write_byte(key[i]);
        
    }
	RF_unselect();
	delay_ms(10);
    writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFE) | (key ? 1:0));
	
}

void setMode(uint8_t newMode)
{
    if (newMode == mode)
    return;

    switch (newMode)
    {
        case RF69_MODE_TX:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
            if (isRFM69HW) setHighPowerRegs(1);
            break;
        case RF69_MODE_RX:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
            if (isRFM69HW) setHighPowerRegs(0);
            break;
        case RF69_MODE_SYNTH:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
            break;
        case RF69_MODE_STANDBY:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
            break;
        case RF69_MODE_SLEEP:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
            break;
        default:
        return;
    }
    // we are using packet mode, so this check is not really needed
    // but waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
    while (mode == RF69_MODE_SLEEP && (readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady
    mode = newMode;
}
    
// internal function
void setHighPowerRegs(uint8_t onOff)
{
    if(onOff==1)
    {
    writeReg(REG_TESTPA1, 0x5D);
    writeReg(REG_TESTPA2, 0x7C);
    }
    else
    {
        writeReg(REG_TESTPA1, 0x55);
        writeReg(REG_TESTPA2, 0x70);
    }
}
    
// for RFM69HW only: you must call setHighPower(1) after rfm69_init() or else transmission won't work
void setHighPower(uint8_t onOff) 
{
    isRFM69HW = onOff;
    writeReg(REG_OCP, isRFM69HW ? RF_OCP_OFF : RF_OCP_ON);

    if (isRFM69HW == 1) // turning ON
        writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
    else
        writeReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | powerLevel); // enable P0 only
}

// get the received signal strength indicator (RSSI)
int16_t readRSSI(uint8_t forceTrigger)
{
    int16_t rssi = 0;
    if (forceTrigger==1)
    {
        // RSSI trigger not needed if DAGC is in continuous mode
        writeReg(REG_RSSICONFIG, RF_RSSI_START);
        while ((readReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00); // wait for RSSI_Ready
    }
    rssi = -readReg(REG_RSSIVALUE);
    rssi >>= 1;
    return rssi;
}

// internal function
//void sendFrame(uint8_t address, uint8_t toAddress, uint8_t opcode, uint8_t extraArg, const void* buffer, uint8_t bufferSize)
void sendFrame(rfHeader * txHeader, const void* buffer)
{
    setMode(RF69_MODE_SLEEP); // turn off receiver to prevent reception while filling fifo
    while ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady
    writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00); // DIO0 is "Packet Sent"
    if (txHeader->rxtxBuffLenght > RF69_MAX_DATA_LEN)
        txHeader->rxtxBuffLenght = RF69_MAX_DATA_LEN;
    // write to FIFO
    RF_select(); //enable data transfer
    SPI0_Write_byte(REG_FIFO | 0x80);
    SPI0_Write_byte(txHeader->rxtxBuffLenght + 4);
    SPI0_Write_byte(txHeader->destinationAddr);
    SPI0_Write_byte(txHeader->senderAddr);
    SPI0_Write_byte(txHeader->opcode);
    SPI0_Write_byte(txHeader->dataCRC);
  
	SPI0_WriteBuff((void *)buffer, txHeader->rxtxBuffLenght);
    RF_unselect(); 
	setMode(RF69_MODE_TX);
	delay_ms(1);
	uint16_t tx_timeout = RF69_TX_LIMIT_MS;
	while (readReg(REG_IRQFLAGS1) != RF_TX_DONE){
		//sprintf(rfDebugStr, "REG_IRQFLAGS1=:%02X; %02d\n\r", readReg(REG_IRQFLAGS1), readReg(REG_IRQFLAGS1) & RF_TX_DONE);
		//DebugSerialWrite(rfDebugStr, sizeof(rfDebugStr));
		tx_timeout--;
		if (tx_timeout==0){
			break;
		}
		delay_us(10);	
	}
	
	//while (readReg(REG_IRQFLAGS1) == 0x00);

	setMode(RF69_MODE_RX);
}

// Calibrate RC
void rcCalibration()
{
    writeReg(REG_OSC1, RF_OSC1_RCCAL_START);
    while ((readReg(REG_OSC1) & RF_OSC1_RCCAL_DONE) == 0x00);
}

uint8_t sendWithRetry(uint8_t toAddress, const void* buffer, uint8_t bufferSize, uint8_t retries, uint8_t retryWaitTime)
{
    for (uint8_t i = 0; i <= retries; i++)
    {
        //send(toAddress, buffer, bufferSize, 1);
        delay_ms(50);
        {
            if (ACKReceived(toAddress)){
                return 1;
            }
        }
    }
    return 0;
}

// should be polled immediately after sending a packet with ACK request
uint8_t ACKReceived(uint8_t fromNodeID)
{
    if (receiveDone())
        return (SENDERID == fromNodeID || fromNodeID == RF69_BROADCAST_ADDR) && ACK_RECEIVED;
    return 0;
}

// checks if a packet was received and/or puts transceiver in receive (ie RX or listen) mode
uint8_t receiveDone()
{
    //cli();

    if (mode == RF69_MODE_RX && PAYLOADLEN > 0)
    {
        setMode(RF69_MODE_STANDBY); // enables interrupts
        return 1;
    }
    else if (mode == RF69_MODE_RX) // already in RX no payload yet
    {
       // sei(); // explicitly re-enable interrupts
        return 0;
    }
    receiveBegin();
    //sei();
    return 0;
}

// internal function
void receiveBegin()
{
    DATALEN = 0;
    SENDERID = 0;
    TARGETID = 0;
    PAYLOADLEN = 0;
    ACK_REQUESTED = 0;
    ACK_RECEIVED = 0;
    RSSI = 0;
    if (readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY)
    writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
    writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01); // set DIO0 to "PAYLOADREADY" in receive mode
    setMode(RF69_MODE_RX);
}

// 1  = disable filtering to capture all frames on network
// 0 = enable node/broadcast filtering to capture only frames sent to this/broadcast address
void promiscuous(uint8_t onOff)
{
    promiscuousMode = onOff;
    if(promiscuousMode==0)
        writeReg(REG_PACKETCONFIG1, (readReg(REG_PACKETCONFIG1) & 0xF9) | RF_PACKET1_ADRSFILTERING_NODEBROADCAST);
    else
        writeReg(REG_PACKETCONFIG1, (readReg(REG_PACKETCONFIG1) & 0xF9) | RF_PACKET1_ADRSFILTERING_OFF);    
}

// Only reenable interrupts if we're not being called from the ISR
void maybeInterrupts()
{
    //if (!inISR) sei();
}

// Enable SPI transfer
void RF_select()
{
    RF_Select();
}

// Disable SPI transfer
void RF_unselect()
{
     RF_Unselect();
}

// Interrupt Service Routine

 rfHeader* data_ready(){
	 
	if (readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY){
		writeReg(REG_OPMODE , RF69_MODE_STANDBY);		 
		RF_select();
		SPI0_Write_byte(REG_FIFO);
		SPI0_ReadBuff(&rfRxHeader, 5);
		rfRxHeader.rxtxBuffLenght -= 5;
		SPI0_ReadBuff(&DATA, rfRxHeader.rxtxBuffLenght);
		RF_unselect();
		writeReg(REG_DIOMAPPING1 , 0x40);
		writeReg(REG_OPMODE , RF69_ListenAbort); 
		rfRxHeader.rssi = readRSSI(0);
		setMode(RF69_MODE_RX);
		rfRxHeader.dataValid=1;
		return &rfRxHeader;
	}else{
		rfRxHeader.dataValid=0;
		return &rfRxHeader;
	}
}


uint8_t simpleCRC(uint8_t *array, uint8_t length) {
	uint8_t checksum = 0;

	for (size_t i = 0; i < length; ++i) {
		checksum += array[i];
	}

	return checksum;
}