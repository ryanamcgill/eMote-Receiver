/*
 *  ======== TM4C_Receiver.c ========
 *
 *  Includes the high level functionality of the TM4C receiver.
 *
 *  Author: Tyler Horton (tch0014@auburn.edu)
 *  Created on: 7/8/16
 *  Last modified: 10/4/16
 */


#include <TM4C_Receiver.h>

/* Structure definitions to set up the CC1101 interface */
static const struct sCC1101 gCC1101Settings = {
	0x29,               // GDO2 output pin configuration
	0x2E,               // GDO1 output pin configuration
	0x06,               // GDO0 output pin configuration
	0x07,               // RXFIFO and TXFIFO thresholds
	0xD3,               // Sync word, high byte
	0x91,               // Sync word, low byte
	PACKET_LEN,			// Packet length
	0x48,               // Packet automation control (PQT set to 2: 0x4-, Append status: 0x-4, CRC: 0x-8)
	0x04,               // Packet automation control (Whitening on: 0x4-, CRC_EN: 0x-4)
	0x00,               // Device address
	LOW_CHANNEL,        // Channel number
	0x06,               // Frequency synthesizer control (for 500k: 0x0E)
	0x00,               // Frequency synthesizer control
	0x22,               // Frequency control word, high byte
	0xB1,               // Frequency control word, middle byte
	0x3B,               // Frequency control word, low byte
	0x99,               // Modem configuration 4 (500k: 0x0E, 25k: 0xC9, increased to 162kHz BW with 0x99)
	0xF8,               // Modem configuration 3 (500k: 0x3B, 25k: 0xF8)
	0x01,               // Modem configuration 2 (15/16 sync word bits: 0x-1, for 500k: 0x71, GFSK with 0x11)
	0xA2,               // Modem configuration 1 (0x22 for no FEC, for 500k: 0xA3)
	0xF8,               // Modem configuration 0 (for 500k: 0x48)
	0x35,               // Modem deviation setting (when FSK modulation is enabled) (for 500k: 0x00)
	0x07,               // Main Radio Control State Machine configuration
	0x0C,               // Main Radio Control State Machine configuration (was 0x0E - stay in RX or TX)
	0x18,               // Main Radio Control State Machine configuration (perhaps needs to be changed)
	0x16,               // Frequency Offset Compensation Configuration (for 500k: 0x1D)
	0x6C,               // Bit synchronization Configuration (for 500k: 0x1C)
	0x43,               // AGC control (for 500k: 0xC7, also has been 0x07)
	0x40,               // AGC control (for 500k: 0x00)
	0x91,               // AGC control (for 500k: 0xB0)
	0x87,               // High byte Event 0 timeout
	0x6B,               // Low byte Event 0 timeout
	0xF8,               // Wake On Radio control
	0x56,               // Front end RX configuration (for 500k: 0xB6)
	0x10,               // Front end TX configuration
	0xE9,               // Frequency synthesizer calibration (for 500k: 0xEA, see note in user guide about fast frequency hopping)
	0x2A,               // Frequency synthesizer calibration
	0x00,               // Frequency synthesizer calibration
	0x1F,               // Frequency synthesizer calibration
	0x41,               // RC oscillator configuration
	0x00,               // RC oscillator configuration
	0x59,               // Frequency synthesizer calibration control
	0x7F,               // Production test
	0x3F,               // AGC test (was 0x3C)
	0x81,               // Various test settings (for 500k: 0x88)
	0x35,               // Various test settings (for 500k: 0x31)
	0x09                // Various test settings
};
struct sCC1101PhyInfo gPhyInfo;
struct sCC1101Spi gSpi = { SpiInit, SpiRead, SpiWrite };
static const struct sCC1101Gdo gGdo0 = {
	Gdo0Init,
	Gdo0Event,
	Gdo0Assert,
	Gdo0Deassert,
	Gdo0State,
	Gdo0Enable
};
static const struct sCC1101Gdo *gGdo[3] = { &gGdo0, NULL, NULL };
//static unsigned char gPaTable[8] = { 0x8E };

/* Function Declarations */
void 	CC1101Init		(void);
void 	Gdo0Interrupt	(void);
void 	ReceivePacket	(void);
void 	ChangeChannel	(void);
void 	ChannelTimeout	(void);
void 	UsbTransmit		(void);
void 	TcpHandler		(void);
int16_t	TcpSendAll		(SOCKET, uint8_t*, int16_t);


/*
 *  ======== main ========
 *
 *  Calls all initialization functions to set up GPIOs, serial USB, and the
 *  CC1101. Starts RTOS BIOS.
 *
 *  Params: none
 *  Return: none
 */
void main(void)
{
	Board_initGeneral();
    Board_initGPIO();
    Board_initUSB(Board_USBDEVICE);
    USBCDCD_init();
    CC1101Init();
    CC1190Init();
    CC1190SetMode(eCC1190ModeRxHGM);
    Board_initEMAC();
    BIOS_start();
}

/*
 *  ======== InitCC1101 ========
 *
 *  Initializes the CC1101 physical information structure. Once this is
 *  initialized, regular operation may begin.
 *
 *  Params: none
 *  Return: none
 */
void CC1101Init(void)
{
	CC1101GdoInit(&gPhyInfo, gGdo);		// Init this one first b/c it will overwrite SPI config
	CC1101SpiInit(&gPhyInfo, &gSpi, NULL);
	CC1101Configure(&gPhyInfo, &gCC1101Settings);
	//CC1101WriteRegisters(&gPhyInfo, CC1101_PATABLE, gPaTable, 8);   // Set the PA table
	CC1101GdoWaitForAssert(gPhyInfo.gdo[0]);
	CC1101GdoEnable(gPhyInfo.gdo[0]);
	CC1101Idle(&gPhyInfo);
	CC1101FlushRxFifo(&gPhyInfo);
	CC1101ReceiverOn(&gPhyInfo);
}

/*
 *  ======= Gdo0Interrupt =======
 *
 *  Handles hardware interrupts from the GDO0 line of the CC1101. These will
 *  occur when the sync word has been received and when the end-of-packet has
 *  been reached.
 *
 *  Params: none
 *  Return: none
 */
void Gdo0Interrupt(void)
{
	if(CC1101GdoGetState(gPhyInfo.gdo[0]) == eCC1101GdoStateWaitForAssert)
	{
		// Sync word received, begin looking for end-of-packet signal.
		CC1101GdoWaitForDeassert(gPhyInfo.gdo[0]);
	}
	else
	{
		// End-of-packet signal has been received.
		CC1101GdoWaitForAssert(gPhyInfo.gdo[0]);
		Swi_post(gdo0_swi);
		GPIO_toggle(Board_LED0);
	}
}

/*
 *  ======= ReceivePacket =======
 *
 *  SWI that reads from the CC1101's FIFO into a packet info structure once the
 *  GDO0 end-of-packet signal has been received. Ensures that there was not an
 *  error with the FIFO.
 *
 *  Params: none
 *  Return: none
 */
void ReceivePacket(void)
{
	struct sPktInfo pkt;

	// Are there bytes in RX FIFO?
	if(CC1101GetRxFifoCount(&gPhyInfo) & 0x7F)
	{
		// Do we have an RX FIFO overflow error?
		if((CC1101GetMarcState(&gPhyInfo) & 0x1F) == eCC1101MarcStateRxfifo_overflow)
		{
			CC1101Idle(&gPhyInfo);
			CC1101FlushRxFifo(&gPhyInfo);
			CC1101ReceiverOn(&gPhyInfo);
		}
		else
		{
			GPIO_toggle(Board_LED1);
			CC1101ReadRxFifo(&gPhyInfo, pkt.data, PACKET_LEN);
			// Send packet via either USB or Ethernet, do not block
			// Packet will be lost if mailbox is full (post returns false in this case)
			Mailbox_post(pkt_mbx, &pkt, BIOS_NO_WAIT);
			// Increment to next channel number
			Semaphore_post(change_channel_sem);
		}
	}
}

/*
 *  ======= ChannelTimeout =======
 *
 *  If a channel timeout has been reached (configured to 1.5s in the RTOS clock
 *  module), it will post a semaphore to change the channel. Using the
 *  semaphore ensures that the channel will not be changed if a packet is
 *  currently being received when the timeout event occurs. It also prevents
 *  the change channel function from being called more than once since it is
 *  also posted in the receive packet function and is a binary semaphore.
 *
 *  Params: none
 *  Return: none
 */
void ChannelTimeout(void)
{
	Semaphore_post(change_channel_sem);
}

/*
 *  ======= ChangeChannel =======
 *
 *	Task that waits to be unblocked by the change channel semaphore and then
 *	increments the CC1101 channel number. Resets the channel timeout clock.
 *
 *	Params: none
 *	Return: none
 */
void ChangeChannel(void)
{
	uint8_t channel = LOW_CHANNEL;  // NORMAL OPERATION
//	num_channels = 5
//	uint8_t channels[num_channels] = {0, 5, 40, 80, 115};
//	uint8_t i = 0;

	for(;;)
	{
		// Wait until unblocked
		Semaphore_pend(change_channel_sem, BIOS_WAIT_FOREVER);
		// Stop the channel timeout
		Clock_stop(channel_timeout);
		// Change the channel
		if(channel == HIGH_CHANNEL) channel = LOW_CHANNEL;
		else channel++;   // NORMAL OPERATION
//		if(i == num_channels-1) i = 0;
//		else i++;
		// Change the channel, clear the FIFO, and the turn receiver back on
		CC1101Idle(&gPhyInfo);
		CC1101SetRegister(&gPhyInfo, CC1101_REG_CHANNR, channel);   // NORMAL OPERATION
//		CC1101SetRegister(&gPhyInfo, CC1101_REG_CHANNR, channels[i]);
		CC1101FlushRxFifo(&gPhyInfo);
		CC1101ReceiverOn(&gPhyInfo);
		// Reset and restart the channel timeout
		Clock_start(channel_timeout);
	}
}

/*
 *  ======= UsbTransmit =======
 *
 *  Task that waits until data has been received by the radio and then sends
 *  the received packet data serially via USB.
 *
 *  Params: none
 *  Return: none
 */
void UsbTransmit(void)
{
	struct sPktInfo pkt;

	if(OUTPUT_MODE == USB_OUTPUT)
	{
		for(;;)
		{
			// Wait until data has been received via the radio
			Mailbox_pend(pkt_mbx, &pkt, BIOS_WAIT_FOREVER);
			// Block while the device is NOT connected to the USB
			USBCDCD_waitForConnect(BIOS_WAIT_FOREVER);
			// Transmit the data
			USBCDCD_sendData(pkt.data, PACKET_LEN, BIOS_WAIT_FOREVER);
		}
	}
}

/*
 *  ======= TcpHandler =======
 *
 *  Task that sets up a connection to a host TCP server at a specified IPv4
 *  address and port. Then the task waits until data has been received by the
 *  radio and then sends the received packet data to the TCP server. If the
 *  connection is lost or a send fails, the task attempts to reconnect to the
 *  server indefinitely.
 *
 *  Params: none
 *  Return: none
 */
void TcpHandler(void)
{
	SOCKET sockfd;
	struct sockaddr_in server_info;
	struct sPktInfo pkt;

	// Fill structure with info on server to connect to
	memset(&server_info, '0', sizeof(server_info));
	server_info.sin_family = AF_INET;
	if(inet_pton(AF_INET, SERVER_IP, &server_info.sin_addr) <= 0) DbgPrintf(DBG_INFO, "inet_pton error\n");
	server_info.sin_port = htons(SERVER_PORT);

	for(;;)
	{
		// Create a new socket
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(sockfd != INVALID_SOCKET)
		{
			// If socket creation is successful attempt to connect to server
			if(connect(sockfd, (PSA)&server_info, sizeof(server_info)) != -1)
			{
				// If connection is great success, wait to send data
				for(;;)
				{
					// Wait until data has been received via the radio
					Mailbox_pend(pkt_mbx, &pkt, BIOS_WAIT_FOREVER);
					// Send data, attempt to reconnect if it fails
					if(TcpSendAll(sockfd, pkt.data, PACKET_LEN) == -1) break;
				}
			}
			else DbgPrintf(DBG_INFO, "connect failed: error = %d\n", fdError());
			// Catch exception from connection or send fails
			if(fdClose(sockfd) == -1) DbgPrintf(DBG_INFO, "socket close failed\n");
		}
		else DbgPrintf(DBG_INFO, "socket creation failed: error = %d\n", fdError());
		System_flush();
	}
}

/*
 *  ======= TcpSendAll =======
 *
 *  Ensures that all of the packet data is sent over the Tcp connection.
 *
 *  Params:
 *  	int16_t - socket file descriptor that is connected to a TCP server
 *  	uint8_t - pointer to the buffer of data to be send
 *  	int16_t - length of the buffer
 *  Return:
 *  	int16_t - 0 on send success, -1 on failure
 */
int16_t TcpSendAll(SOCKET s, uint8_t *buf, int16_t len)
{
	int16_t total = 0;			// How many bytes we've sent
	int16_t bytesleft = len;	// How many left to send
	int16_t n;

	while(total < len)
	{
		n = send(s, buf+total, bytesleft, 0);
		if(n == -1) break;
		total += n;
		bytesleft -= n;
	}
	return(n == -1 ? -1 : 0);
}

/*
 *  ======= netIPAddrHook =======
 *
 *  Called once the device has obtained an IP address. This task sets up the
 *  TCP handler task.
 *
 *  Params: none
 *  Return: none
 */
void netIPAddrHook(void)
{
	static bool task_created = false;
    Task_Handle taskHandle;
    Task_Params taskParams;
    Error_Block eb;

	if((OUTPUT_MODE == ETH_OUTPUT) && !task_created)
	{
		Error_init(&eb);				// Make sure Error_Block is initialized
		Task_Params_init(&taskParams);
		taskParams.stackSize = 1024;
		taskParams.priority = 1;
		taskHandle = Task_create((Task_FuncPtr)TcpHandler, &taskParams, &eb);
		if(taskHandle == NULL) System_printf("netIPAddrHook: Failed to create TcpHandler\n");
		else task_created = true;
		System_flush();
    }
}
