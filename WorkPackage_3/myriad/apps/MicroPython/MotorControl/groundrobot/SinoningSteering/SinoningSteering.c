/**
 * @file      SinoningSteering.c
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 31 May 2016
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include "SinoningSteering.h"

// 2: Global variables
// ----------------------------------------------------------------------------

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

/**
 * Store the possible actions of the robot
 */
groundrobot_actions_t SinoningSteering_actions = { .forward = SinoningSteeringForward, .backward =
        SinoningSteeringBackward, .tankturn = SinoningSteeringTankTurn, .stop = SinoningSteeringStop };

/**
 * Store the id of the pins which are going to be used to control the Cherokey 4WD
 */
uint32 SinoningSteering_pin_direction;
uint32 SinoningSteering_pin_direction_inv;
uint32 SinoningSteering_pin_speed;
uint32 SinoningSteering_pin_angle;

/**
 * Store the pin state which will cause the car to move forward
 */
uint8 SinoningSteering_pin_state_forward;

/**
 * Store the value which sets the servo in the neutral position
 */
uint64 SinoningSteering_servo_neutral_count;

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

void SinoningSteeringSetup(uint32 pin_direction, uint32 pin_direction_inv, uint32 pin_speed, uint32 pin_angle, uint32 pin_state_forward)
{
    // Store the pin ids
    SinoningSteering_pin_direction = pin_direction;
    SinoningSteering_pin_direction_inv = pin_direction_inv;
    SinoningSteering_pin_speed = pin_speed;
    SinoningSteering_pin_angle = pin_angle;
    SinoningSteering_pin_state_forward = pin_state_forward;

    // Set the enable pins as output
    pinMode(SinoningSteering_pin_direction, OUTPUT);

    // Setup the pwm pins
    analogWrite(SinoningSteering_pin_speed, 128);

    // Setup the servo
    ServoAttach(pin_angle);

    // Update the period of the pwm to 30ms
    ServoSetPeriod(pin_angle, 3000);

    // Store the count for the neutral position
    SinoningSteering_servo_neutral_count = 100;

    // Set the wheels to neutral position
    ServoWriteMicroseconds(pin_angle, SinoningSteering_servo_neutral_count);

    // Stop the car
    SinoningSteeringStop();
}

void SinoningSteeringSetDirection(uint32 direction)
{
    // Set the direction on the back wheels
    digitalWrite(SinoningSteering_pin_direction, direction);
    digitalWrite(SinoningSteering_pin_direction_inv, !direction);
}

void SinoningSteeringSetAngleSpeed(sint32 angle, uint32 speed)
{
    // Validate the speed
    if (speed > SINONINGSTEERING_MAX_SPEED)
    {
        // Set the speed to the maximum value
        speed = SINONINGSTEERING_MAX_SPEED;
    }

    // Set the speed
    analogWrite(SinoningSteering_pin_speed, speed);

    // Set the angle
    ServoWriteMicroseconds(SinoningSteering_pin_angle, SinoningSteering_servo_neutral_count + angle);
}

void SinoningSteeringStop()
{
    // Set the direction to front
    SinoningSteeringSetDirection(SinoningSteering_pin_state_forward);

    // Set the angle and speed to zero
    SinoningSteeringSetAngleSpeed(0, 0);
}

void SinoningSteeringForward(sint32 angle, uint32 speed)
{
    // Set the direction to front
    SinoningSteeringSetDirection(SinoningSteering_pin_state_forward);

    // Set the angle and speed
    SinoningSteeringSetAngleSpeed(angle, speed);
}

void SinoningSteeringBackward(sint32 angle, uint32 speed)
{
    // Set the direction to backwards
    SinoningSteeringSetDirection(!SinoningSteering_pin_state_forward);

    // Set the angle and speed
    SinoningSteeringSetAngleSpeed(angle, speed);
}

void SinoningSteeringTankTurn(sint32 angle, uint32 speed)
{
    // Do nothing
}

