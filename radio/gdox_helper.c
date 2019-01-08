/*
 *  ======= gdox_helper.c =======
 *
 *	Includes the low-level helper functions for the CC1101 library that allow
 *	the library to handle signals on the GDOx lines.
 *
 *	Author: Tyler Horton (tch0014@auburn.edu)
 *  Created on: 6/29/16
 *  Last modified: 8/16/16
 */


#include <radio/radio_driver.h>

bool gdo0_wait_for_assert;


/*
 *  ======= Gdo0Init =======
 *
 *	Initializes the interrupt and callback function for the CC1101 GDO0 line.
 *	Uses modified board configuration arrays in EK_TM4C1294XL.c (gpioPinConfigs
 *	and gpioCallbackFunctions) for the GPIO initialization.
 *
 *	Params: none
 *	Return: none
 */
void Gdo0Init(void)
{
	Board_initGPIO();								                // Modified to include initialization for GDO0
	GPIO_setCallback(CC1101_GDO0, (GPIO_CallbackFxn)Gdo0Interrupt); // Maps the interrupt to the Gdo0Interrupt function in main.c
}

/*
 *  ======= Gdo0Event =======
 *
 *	Checks if the interrupt was from the GDO0 line. However, since the
 *	interrupt is handled through callback functionality of RTOS, it is always
 *	true, so no checking is needed.
 *
 *	Params: char - interrupt event register for interrupt port
 *	Returns: bool - true if interrupt was from GDO0 line
 */
bool Gdo0Event(unsigned char event)
{
	return(true);
}

/*
 *  ======= Gdo0Assert =======
 *
 *  Sets the GPIO interrupt for the GDO0 line to rising-edge triggered.
 *
 *  Params: none
 *  Return: none
 */
void Gdo0Assert(void)
{
	gdo0_wait_for_assert = true;
	GPIO_disableInt(CC1101_GDO0);
	GPIO_clearInt(CC1101_GDO0);
	GPIO_setConfig(CC1101_GDO0, (GPIO_CFG_IN_INT_ONLY | GPIO_CFG_IN_INT_RISING));	// Look for low-to-high transition
	GPIO_enableInt(CC1101_GDO0);
}

/*
 *  ======= Gdo0Deassert =======
 *
 *  Sets the GPIO interrupt for the GDO0 line to falling-edge triggered.
 *
 *  Params: none
 *  Return: none
 */
void Gdo0Deassert(void)
{
	gdo0_wait_for_assert = false;
	GPIO_disableInt(CC1101_GDO0);
	GPIO_clearInt(CC1101_GDO0);
	GPIO_setConfig(CC1101_GDO0, (GPIO_CFG_IN_INT_ONLY | GPIO_CFG_IN_INT_FALLING));   // Look for high-to-low transition
	GPIO_enableInt(CC1101_GDO0);
}

/*
 *  ======= Gdo0State =======
 *
 *  Checks if the GPIO interrupt for the GDO0 line is currently rising-edge or
 *  falling-edge triggered.
 *
 *  Params: none
 *  Return: eCC1101GdoState - eCC1101GdoStateWaitForAssert if rising edge,
 *  						  eCC1101GdoStateWaitForDeassert if falling edge
 */
enum eCC1101GdoState Gdo0State(void)
{
	return(gdo0_wait_for_assert ? eCC1101GdoStateWaitForAssert : eCC1101GdoStateWaitForDeassert);
}


/*
 *  ======= Gdo0Enable =======
 *
 *	Enables or disables the GPIO interrupt for the GDO0 line.
 *
 *	Params: bool - true to enable, false to disable
 *	Return: none
 */
void Gdo0Enable(bool en)
{
	GPIO_clearInt(CC1101_GDO0);
	if(en) GPIO_enableInt(CC1101_GDO0);
	else GPIO_disableInt(CC1101_GDO0);
}
