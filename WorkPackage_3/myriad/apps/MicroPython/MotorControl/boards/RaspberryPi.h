/**
 * @file      Cherokey4WDRaspberryPi.h
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 21 Oct 2015
 * Author: dexmont
 *
 * @defgroup GroupName Group title
 * @{
 * @brief     Group briefing
 *
 * Detailed description
 */

#ifndef _CHEROKEY4WD_CHEROKEY4WDRASPBERRYPI_H_
#define _CHEROKEY4WD_CHEROKEY4WDRASPBERRYPI_H_

#ifdef RASPBERRY_PI

// 1: Includes
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// Define used for debug purposes
#define CHEROKEY4WDRASPBERRYPI_DEBUG  1

// Define states of the pins
#define LOW     0
#define HIGH    1

// Redefine the types
typedef unsigned char uint8;
typedef signed char sint8;

// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

/**
 * Sets the specified pin as output
 * @param pint_id
 */
void setPinAsOutput(uint8 pint_id);

/**
 * Sets the specified pin at the given state
 * @param pin_id
 * @param state
 */
void setPinState(uint8 pin_id, uint8 state);

/**
 * Sets the specified pin as PWM output
 * @param pin_id
 * @param pwm_count The count of ticks for a pwm cycle
 */
void setPinAsPWM(uint8 pin_id, uint32 pwm_count = 0);

/**
 * Sets the duty cycle for the specified PWM pin
 * @param pin_id
 * @param duty_cyle
 */
void setPWMDutyCycle(uint8 pin_id, uint8 duty_cyle);

#endif

#endif /* _CHEROKEY4WD_CHEROKEY4WDRASPBERRYPI_H_ */

/**
 * @}
 */
