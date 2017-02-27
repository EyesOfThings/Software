/**
 * @file      SinoningSteering.h
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

#ifndef GROUNDROBOT_SINONINGSTEERING_SINONINGSTEERING_H_
#define GROUNDROBOT_SINONINGSTEERING_SINONINGSTEERING_H_


// 1: Includes
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include "types.h"
#include "groundrobot/groundrobot.h"
#include "boards/Boards.h"

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// Set the maximum value for the speed
#define SINONINGSTEERING_MAX_SPEED   255

// Set the maximum value for the angle
#define SINONINGSTEERING_MAX_ANGLE   255

// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

/**
 * Declare the Cherokey4WD actions
 */
extern groundrobot_actions_t SinoningSteering_actions;

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

/**
 * Setup the pins used to control the Sinoning Steering car. Set the pin state corresponding to the forward direction (HIGH or LOW)
 * @param pin_direction Pin used for setting the direction (forward of backward)
 * @param pin_direction_inv Pin to set as inverse of pin_direction
 * @param pin_speed Pin used to set the speed of the car
 * @param pin_angle Pin used to set the angle of the car
 * @param pin_state_forward Pin state which moves the car forward
 */
void SinoningSteeringSetup(uint32 pin_direction, uint32 pin_direction_inv, uint32 pin_speed, uint32 pin_angle, uint32 pin_state_forward);

/**
 * Stop the car
 */
void SinoningSteeringStop();

/**
 * Move set the car wheel to move at the specified angle and speed
 * @param angle Angle of the turn, a value of zero indicates move in straight line. The Sinoningsteering car has a servo for the angle, negative angles are clock ticks subtracted to the neutral position, positive values are clock ticks added to the neutral position.
 * @param speed Speed of the wheels, 0 to 255.
 */
void SinoningSteeringForward(sint32 angle, uint32 speed);

/**
 * Set the car wheels to move to the front at the specified angle and speed
  * @param angle Angle of the turn, a value of zero indicates move in straight line. The Sinoningsteering car has a servo for the angle, negative angles are clock ticks subtracted to the neutral position, positive values are clock ticks added to the neutral position.
 * @param speed Speed of the wheels, 0 to 255.
 */
void SinoningSteeringBackward(sint32 angle, uint32 speed);

/**
 * Empty function set for compatibility with groundrobot_actions_t structure.
 * @param angle Set to zero
 * @param speed Set to zero
 */
void SinoningSteeringTankTurn(sint32 angle, uint32 speed);

// ----------------------------------------- Helper functions (used as private methods in a class  -------------------------------------------

/**
 * Set the angle and speed of the car. The Cherokey has differential steering.
 * @param angle Angle of the turn, a value of zero indicates move in straight line. The Sinoningsteering car has a servo for the angle, negative angles are clock ticks subtracted to the neutral position, positive values are clock ticks added to the neutral position.
 * @param speed Speed of the wheels, 0 to 255.
 */
void SinoningsteeringSetAngleSpeed(sint32 angle, uint32 speed);

/**
 * Set the direction pins
 * @param direction    Direction of the wheels (forwards or backwards)
 */
void SinoningSteeringSetDirection(uint32 direction);



#endif /* GROUNDROBOT_SINONINGSTEERING_SINONINGSTEERING_H_ */

/**
 * @}
 */
