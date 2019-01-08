/*
 *  ======= cc1190_helper.c =======
 *
 *  Includes the low-level helper functions to control the CC1190 LNA modes.
 *  The control of the CC1190 is based on the following table:
 *
 *  PA_EN   LNA_EN  HGM     Mode
 *  *****   ******  ***     ****
 *  0       0       X       Power Down
 *  0       1       0       RX LGM
 *  0       1       1       RX HGM
 *  1       0       0       TX LGM
 *  1       0       1       Tx HGM
 *
 *  Author: Tyler Horton (tch0014@auburn.edu)
 *  Created on: 8/3/17
 *  Last modified: 8/3/17
 */


#include "radio_driver.h"

#define PA_EN_PORT  GPIO_PORTQ_BASE
#define PA_EN_PIN   GPIO_PIN_1
#define LNA_EN_PORT GPIO_PORTP_BASE
#define LNA_EN_PIN  GPIO_PIN_1
#define HGM_PORT    GPIO_PORTP_BASE
#define HGM_PIN     GPIO_PIN_0

void    CC1190Init              (void);
void    CC1190SetMode           (enum eCC1190Mode mode);
void    CC1190SetControlPins    (uint8_t pa_en, uint8_t lna_en, uint8_t hgm);


void CC1190Init(void)
{
    SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
    GPIOPinTypeGPIOOutput(PA_EN_PORT, PA_EN_PIN);
    GPIOPinTypeGPIOOutput(LNA_EN_PORT, LNA_EN_PIN);
    GPIOPinTypeGPIOOutput(HGM_PORT, HGM_PIN);
}

void CC1190SetMode(enum eCC1190Mode mode)
{
    switch(mode)
    {
        case eCC1190ModePowerDown:
            CC1190SetControlPins(0x00, 0x00, 0x00);
            break;
        case eCC1190ModeRxLGM:
            CC1190SetControlPins(0x00, 0xFF, 0x00);
            break;
        case eCC1190ModeRxHGM:
            CC1190SetControlPins(0x00, 0xFF, 0xFF);
            break;
        case eCC1190ModeTxLGM:
            CC1190SetControlPins(0xFF, 0x00, 0x00);
            break;
        case eCC1190ModeTxHGM:
            CC1190SetControlPins(0xFF, 0x00, 0xFF);
            break;
        default:
            CC1190SetControlPins(0x00, 0x00, 0x00);
    }
}

void CC1190SetControlPins(uint8_t pa_en, uint8_t lna_en, uint8_t hgm)
{
    GPIOPinWrite(PA_EN_PORT, PA_EN_PIN, pa_en);
    GPIOPinWrite(LNA_EN_PORT, LNA_EN_PIN, lna_en);
    GPIOPinWrite(HGM_PORT, HGM_PIN, hgm);
}
