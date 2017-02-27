/**
 * @file      ground_robot.h
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

#ifndef MICROPYTHON_EOT_GROUND_ROBOT_H_
#define MICROPYTHON_EOT_GROUND_ROBOT_H_

// 1: Includes
// ----------------------------------------------------------------------------

#include "py/nlr.h"
#include "py/runtime.h"
#include "groundrobot/groundrobot.h"
#include "groundrobot/Cherokey4WD/Cherokey4WD.h"
#include "groundrobot/SinoningSteering/SinoningSteering.h"
#include <stdio.h>
#include <string.h>

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

/**
 * Set the cont of currently supported robots
 */
#define ROBOT_COUNT     2

// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

/**
 * Create an enumeration for the types of supported robots
 */
typedef enum
{
    Cherokey4WD,            // Set the robot as Cherokey4WD
    SinoningSteering,       // Set the robot as Sinoning Steering
    Unknown,                // Set the robot as unknown
} eot_groundrobot_type_t;

/**
 * Create the GroundRobot object
 */
typedef struct
{
        const mp_obj_base_t base;       // Base type of the object
        eot_groundrobot_type_t type;    // Store the robot type
        qstr name;                      // Store the robot name
        groundrobot_actions_t *actions; // Store the possible actions of the robot
} eot_groundrobot_obj_t;

/**
 * Create a reference to the pin type
 */
extern const mp_obj_type_t eot_groundrobot_type;

#endif /* MICROPYTHON_EOT_GROUND_ROBOT_H_ */

/**
 * @}
 */
