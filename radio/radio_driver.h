/*
 *  ======= cc1101Driver.h =======
 *
 *  Adds some additional files that the low-level CC1101 drivers need and
 *  includes the driver function definitions.
 *
 *	Author: Tyler Horton (tch0014@auburn.edu)
 *  Created on: 6/27/16
 *  Last modified: 8/16/16
 */


#ifndef RADIO_DRIVER_H_
#define RADIO_DRIVER_H_

#include "../TM4C_Receiver.h"
#include "CC1101.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"

enum eCC1190Mode
{
  eCC1190ModePowerDown  = 0x00u,
  eCC1190ModeRxLGM      = 0x01u,
  eCC1190ModeRxHGM      = 0x02u,
  eCC1190ModeTxLGM      = 0x03u,
  eCC1190ModeTxHGM      = 0x04u
};

void SpiInit(void);
void SpiWrite(unsigned char address, const unsigned char *pBuffer, unsigned char count);
void SpiRead(unsigned char address, unsigned char *pBuffer, unsigned char count);

void Gdo0Init(void);
bool Gdo0Event(unsigned char event);
void Gdo0Assert(void);
void Gdo0Deassert(void);
enum eCC1101GdoState Gdo0State(void);
void Gdo0Enable(bool en);

void CC1190Init(void);
void CC1190SetMode(enum eCC1190Mode mode);

#endif /* RADIO_DRIVER_H_ */
