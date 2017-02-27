/**
 * @file      Cherokey4WDRaspberryPi.cpp
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 21 Oct 2015
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include <boards/RaspberryPi.h>

#ifdef RASPBERRY_PI

// 2: Global variables
// ----------------------------------------------------------------------------

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

/**
 * Sets the specified pin as output
 * @param pint_id
 */
void setPinAsOutput(uint8 pint_id)
{
    // Set the pin mode
    pinMode(pint_id, OUTPUT);
}

/**
 * Sets the specified pin at the given state
 * @param pin_id
 * @param state
 */
void setPinState(uint8 pin_id, uint8 state)
{
    // Set the pin to the specified state
    digitalWrite(pin_id, state);

#ifdef BOARD_DEBUG
    printf("setting pin %d to state %d\n", pin_id, state);
#endif
}

/**
 * Sets the specified pin as PWM output
 * @param pin_id
 * @param pwm_count The count of ticks for a pwm cycle
 */
void setPinAsPWM(uint8 pin_id, uint32 pwm_count)
{
    // Set the pin as pwm
    softPwmCreate(pin_id, 0, 255);
}

/**
 * Sets the duty cycle for the specified PWM pin, the duty cycle is a value between 0 and 255
 * @param pin_id
 * @param duty_cyle
 */
void setPWMDutyCycle(uint8 pin_id, uint8 duty_cyle)
{
    // Set the duty cycle of the pwm
    softPwmWrite(pin_id, duty_cyle);
}

#endif
