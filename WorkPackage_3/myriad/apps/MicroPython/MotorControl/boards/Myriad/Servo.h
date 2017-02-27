/**
 * @file      Servo.h
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 7 Jun 2016
 * Author: dexmont
 *
 * @defgroup GroupName Group title
 * @{
 * @brief     Group briefing
 *
 * Detailed description
 */

#ifndef BOARDS_MYRIAD_SERVO_H_
#define BOARDS_MYRIAD_SERVO_H_

#ifdef MYRIAD

// 1: Includes
// ----------------------------------------------------------------------------

#include "Myriad.h"

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// Set the maximum number of servos we can control
#define SERVO_MAX_COUNT             5

// Set the default period for a servo
#define SERVO_PERIOD_DEFAULT        1000 // Units of 10 us

// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

/**
 * Attach the servo to the specified pin_id
 * @param pin_id
 */
void ServoAttach(uint32 pin_id);

/**
 * Release the pin_id from the servo controller
 * @param pin_id
 */
void ServoDetach(uint32 pin_id);

/**
 * Set the period for the pwm associated to the servo.
 */
void ServoSetPeriod(uint32 pin_id, uint64 period);

/**
 * Writes a value (time in high) in microseconds (us) to the servo. On a standard servo this sets the angle of the shaft.
 * @param pin_id    Pin id to set
 * @param us    Value in microseconds to set.
 */
void ServoWriteMicroseconds(uint32 pin_id, uint64 us);

#endif

#endif /* BOARDS_MYRIAD_SERVO_H_ */

/**
 * @}
 */
