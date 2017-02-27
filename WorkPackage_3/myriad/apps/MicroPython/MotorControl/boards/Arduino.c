/**
 * @file      Cherokey4WDArduino.c
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 27 Oct 2015
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------

#include <boards/Arduino.h>

#ifdef ARDUINO

// 2: Global variables
// ----------------------------------------------------------------------------

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

/**
 * Sets the specified pin as output
 * @param pint_id
 */
void setPinAsOutput(unsigned char pin_id)
{
    // Set the pin mode
    pinMode(pin_id, OUTPUT);
}

/**
 * Sets the specified pin at the given state
 * @param pin_id
 * @param state
 */
void setPinState(unsigned char pin_id, unsigned char state)
{
    // Set the pin to the specified state
    digitalWrite(pin_id, state);
}

/**
 * Sets the specified pin as PWM output
 * @param pin_id
 * @param pwm_count
 */
void setPinAsPWM(unsigned char pin_id, unsigned int pwm_count)
{
    // Set the pin mode
    pinMode(pin_id, OUTPUT);
}

/**
 * Sets the duty cycle for the specified PWM pin, the duty cycle is a value between 0 and 255
 * @param pin_id
 * @param duty_cyle
 */
void setPWMDutyCycle(unsigned char pin_id, unsigned char duty_cyle)
{
    // Set the duty cycle of the pwm
    analogWrite(pin_id, duty_cyle);
}

#endif

