/**
 * @file      Cherokey4WD.h
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 31 May 2016
 * Author: dexmont
 *
 * @defgroup GroupName Group title
 * @{
 * @brief     Group briefing
 *
 * Detailed description
 */

#ifndef GROUNDROBOT_CHEROKEY4WD_CHEROKEY4WD_H_
#define GROUNDROBOT_CHEROKEY4WD_CHEROKEY4WD_H_

// 1: Includes
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include "types.h"
#include "groundrobot/groundrobot.h"
#include "boards/Boards.h"

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// Set the maximum value for the speed
#define CHEROKEY4WD_MAX_SPEED   255

// Set the maximum value for the angle
#define CHEROKEY4WD_MAX_ANGLE   255

// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

/**
 * Declare the Cherokey4WD actions
 */
extern groundrobot_actions_t Cherokey4WD_actions;

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

/**
 * Set up the pins which are going to be used to control the Cherokey4WD.
 * Set the pin state which corresponds to the forward direction (HIGH or LOW)
 * @param pin_m1_en
 * @param pin_m1_pwm
 * @param pin_m2_en
 * @param pin_m2_pwm
 * @param pin_state_forward
 */
void Cherokey4WDSetup(uint32 pin_m1_en, uint32 pin_m1_pwm, uint32 pin_m2_en, uint32 pin_m2_pwm, uint32 pin_state_forward);

/**
 * Stop the car
 */
void Cherokey4WDStop();

/**
 * Move set the car wheel to move at the specified angle and speed
 * @param angle Angle of the turn, a value of zero indicates move in straight line. The Cherokey4WD has differential steering, negative angles are subtracted to the speed of the wheels on the left side, positive values are subtracted to the speed of the wheels on the right side.
 * @param speed Speed of the wheels, 0 to 255.
 */
void Cherokey4WDForward(sint32 angle, uint32 speed);

/**
 * Set the car wheels to move to the front at the specified angle and speed
 * @param angle Angle of the turn, a value of zero indicates move in straight line. The Cherokey4WD has differential steering, negative angles are subtracted to the speed of the wheels on the left side, positive values are subtracted to the speed of the wheels on the right side.
 * @param speed Speed of the wheels, 0 to 255.
 */
void Cherokey4WDBackward(sint32 angle, uint32 speed);

/**
 * Set the car wheels to move as tank at the specified angle and speed
 * @param angle Angle of the turn, a value of zero indicates move in straight line, negative values move to the left and positive values move to the right
 * @param speed Speed of the wheels, 0 to MAX_SPEED
 */
void Cherokey4WDTankTurn(sint32 angle, uint32 speed);

// ----------------------------------------- Helper functions (used as private methods in a class  -------------------------------------------

/**
 * Set the angle and speed of the car. The Cherokey has differential steering.
 * @param angle Angle of the turn, a value of zero indicates move in straight line. The Cherokey4WD has differential steering, negative angles are subtracted to the speed of the wheels on the left side, positive values are subtracted to the speed of the wheels on the right side.
 * @param speed Speed of the wheels, 0 to 255.
 */
void Cherokey4WDSetAngleSpeed(sint32 angle, uint32 speed);

/**
 * Set the direction pins
 * @param direction_left    Direction of the left wheels
 * @param direction_right   Direction of the right wheels
 */
void Cherokey4WDSetDirection(uint32 direction_left, uint32 direction_right);

#endif /* GROUNDROBOT_CHEROKEY4WD_CHEROKEY4WD_H_ */

/**
 * @}
 */
