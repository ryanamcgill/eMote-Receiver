/*
 *  ======= TM4C_Receiver.h =======
 *
 *	Has the main includes for the project and has some defines and sturcture
 *	definitions. Also has any functions needed for cross-project access.
 *
 *	Author: Tyler Horton (tch0014@auburn.edu)
 *  Created on: 7/8/16
 *  Last modified: 10/4/16
 */


#ifndef TM4C_RECEIVER_H_
#define TM4C_RECEIVER_H_

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Timer.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOTiva.h>
#include <ti/drivers/SPI.h>

/* NDK support */
#include <netmain.h>
#include <os/oskern.h>

/* Various Drivers */
#include "Board.h"
#include "radio/radio_driver.h"
#include "usb_drivers/USBCDCD.h"

/* General Defines */
#define PACKET_LEN		10	// Make sure to also change the mailbox message size in the config file
#define LOW_CHANNEL	    0	// Low end of channels to scan over
#define HIGH_CHANNEL	0	// High end of channels to scan over
/* Output Mode Defines */
#define USB_OUTPUT		1
#define ETH_OUTPUT		0
#define OUTPUT_MODE		USB_OUTPUT	// Change the method of sending the packets to a computer
/* TCP Defines */
#define SERVER_IP		"192.168.1.100"	// IP address of the TCP server to connect to
#define SERVER_PORT		9000			// Port that the TCP server is listening on

/* Structures */
struct sPktInfo
{
	uint8_t data[PACKET_LEN];
};

/* Functions */
void 	CC1101Init		(void);
void 	Gdo0Interrupt	(void);
void 	ReceivePacket	(void);
void 	ChangeChannel	(void);
void 	ChannelTimeout	(void);
void 	UsbTransmit		(void);
void 	TcpHandler		(void);
int16_t	TcpSendAll		(SOCKET, uint8_t*, int16_t);

#endif /* TM4C_RECEIVER_H_ */
