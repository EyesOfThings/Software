/**
 * @file      Servo.c
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 7 Jun 2016
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include "Servo.h"
#include "Myriad.h"

// 2: Global variables
// ----------------------------------------------------------------------------

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

void ServoAttach(uint32 pin_id)
{
    // Setup the pin as pwm
    registerPinSWPWM(pin_id, SERVO_PERIOD_DEFAULT);
}

void ServoDetach(uint32 pin_id)
{
    // TODO add code to free the pin
}

void ServoSetPeriod(uint32 pin_id, uint64 period)
{
    // Update the pwm period
    setPWMPeriod(pin_id, period);
}

void ServoWriteMicroseconds(uint32 pin_id, uint64 us)
{
    // Set the ime on high for the servo
    setPWMTimeHigh(pin_id, us);
}

