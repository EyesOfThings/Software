/**
 * @file      Cherokey4WDArduino.h
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 27 Oct 2015
 * Author: dexmont
 *
 * @defgroup GroupName Group title
 * @{
 * @brief     Group briefing
 *
 * Detailed description
 */

#ifndef CHEROKEY4WD_CHEROKEY4WDARDUINO_H_
#define CHEROKEY4WD_CHEROKEY4WDARDUINO_H_

#ifdef ARDUINO

// 1: Includes
// ----------------------------------------------------------------------------

#include "Arduino.h"
#include "Cherokey4WDArduinoTypes.h"


// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// Define states of the pins
#define LOW     0
#define HIGH    1

// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

/**
 * Sets the specified pin as output
 * @param pin_id
 */
void setPinAsOutput(unsigned char pin_id);

/**
 * Sets the specified pin at the given state
 * @param pin_id
 * @param state
 */
void setPinState(unsigned char pin_id, unsigned char state);

/**
 * Sets the specified pin as PWM output
 * @param pin_id
 * @param pwm_count The count to ticks for a full cycle
 */
void setPinAsPWM(unsigned char pin_id, unsigned int pwm_count = 0);

/**
 * Sets the duty cycle for the specified PWM pin
 * @param pin_id
 * @param duty_cyle
 */
void setPWMDutyCycle(unsigned char pin_id, unsigned char duty_cyle);


#endif

#endif /* CHEROKEY4WD_CHEROKEY4WDARDUINO_H_ */

/**
 * @}
 */
