/**
 * @file      Cherokey4WD.c
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 31 May 2016
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include "Cherokey4WD.h"

// 2: Global variables
// ----------------------------------------------------------------------------

/**
 * Store the possible actions of the robot
 */
groundrobot_actions_t Cherokey4WD_actions = { .forward = Cherokey4WDForward, .backward = Cherokey4WDBackward,
    .tankturn = Cherokey4WDTankTurn, .stop = Cherokey4WDStop };

/**
 * Store the id of the pins which are going to be used to control the Cherokey 4WD
 */
uint32 Cherokey4WD_pin_m1_enable;
uint32 Cherokey4WD_pin_m1_pwm;
uint32 Cherokey4WD_pin_m2_enable;
uint32 Cherokey4WD_pin_m2_pwm;

/**
 * Store the pin state which will cause the car to move forward
 */
uint8 Cherokey4WD_pin_state_forward;

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

void Cherokey4WDSetup(uint32 pin_m1_en, uint32 pin_m1_pwm, uint32 pin_m2_en, uint32 pin_m2_pwm, uint32 pin_state_forward)
{
    // Store the pin ids
    Cherokey4WD_pin_m1_enable = pin_m1_en;
    Cherokey4WD_pin_m1_pwm = pin_m1_pwm;
    Cherokey4WD_pin_m2_enable = pin_m2_en;
    Cherokey4WD_pin_m2_pwm = pin_m2_pwm;
    Cherokey4WD_pin_state_forward = pin_state_forward;

    // Set the enable pins as output
    pinMode(Cherokey4WD_pin_m1_enable, OUTPUT);
    pinMode(Cherokey4WD_pin_m2_enable, OUTPUT);

    // Setup the pwm pins
    analogWrite(Cherokey4WD_pin_m1_pwm, 0);
    analogWrite(Cherokey4WD_pin_m2_pwm, 0);

    // Stop the car
    Cherokey4WDStop();
}

void Cherokey4WDSetDirection(uint32 direction_left, uint32 direction_right)
{
    // Set the direction on the left wheels
    digitalWrite(Cherokey4WD_pin_m1_enable, direction_left);

    // Set the direction on the right wheels (negated as the Cherokey4WD does some processing and reverses them.
    // Allows to keep the same polarity on the motor pins)
    digitalWrite(Cherokey4WD_pin_m2_enable, !direction_right);
}

void Cherokey4WDSetAngleSpeed(sint32 angle, uint32 speed)
{
    uint8 speed_left, speed_right;

    // Validate the speed
    if (speed > CHEROKEY4WD_MAX_SPEED)
    {
        // Set the speed to the maximum value
        speed = CHEROKEY4WD_MAX_SPEED;
    }

    // Test if the angle is negative (the car should turn to the left)
    if (angle < 0)
    {
        // Remove the sign
        angle = -angle;

        // Validate the angle
        if ((uint32) angle > speed)
        {
            // Set the angle to the maximum (should correspond to a 180 degrees turn)
            angle = speed;
        }

        // Subtract the angle from the speed
        speed_left = speed;
        speed_right = speed - angle;
    }
    else
    {
        // Validate the angle
        if ((uint32) angle > speed)
        {
            // Set the angle to the maximum (should correspond to a 180 degrees turn)
            angle = speed;
        }

        // Subtract the angle from the speed
        speed_left = speed - angle;
        speed_right = speed;

    }

    // Set the speed on the left and right wheels
    analogWrite(Cherokey4WD_pin_m1_pwm, speed_left);
    analogWrite(Cherokey4WD_pin_m2_pwm, speed_right);
}

void Cherokey4WDStop()
{
    // Set the direction to front
    Cherokey4WDSetDirection(Cherokey4WD_pin_state_forward, Cherokey4WD_pin_state_forward);

    // Set the angle and speed to zero
    Cherokey4WDSetAngleSpeed(0, 0);
}

void Cherokey4WDForward(sint32 angle, uint32 speed)
{
    // Set the direction to front
    Cherokey4WDSetDirection(Cherokey4WD_pin_state_forward, Cherokey4WD_pin_state_forward);

    // Set the angle and speed
    Cherokey4WDSetAngleSpeed(angle, speed);
}

void Cherokey4WDBackward(sint32 angle, uint32 speed)
{
    // Set the direction to backwards
    Cherokey4WDSetDirection(!Cherokey4WD_pin_state_forward, !Cherokey4WD_pin_state_forward);

    // Set the angle and speed
    Cherokey4WDSetAngleSpeed(angle, speed);
}

/**
 * Set the car wheels to move as tank at the specified angle and speed
 * @param angle Angle of the turn, a value of zero indicates move in straight line, negative values move to the left and positive values move to the right
 * @param speed Speed of the wheels
 */
void Cherokey4WDTankTurn(sint32 angle, uint32 speed)
{
    // Test the direction of the turn
    if (angle < 0)
    {
        // Set the direction as opposite
        Cherokey4WDSetDirection(Cherokey4WD_pin_state_forward, !Cherokey4WD_pin_state_forward);
    }
    else
    {
        // Set the direction as opposite
        Cherokey4WDSetDirection(!Cherokey4WD_pin_state_forward, Cherokey4WD_pin_state_forward);
    }

    // Set the angle and speed
    Cherokey4WDSetAngleSpeed(angle, speed);
}

