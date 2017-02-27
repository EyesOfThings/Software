/**
 * @file      groundrobot.h
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

#ifndef GROUNDROBOT_GROUNDROBOT_H_
#define GROUNDROBOT_GROUNDROBOT_H_

// 1: Includes
// ----------------------------------------------------------------------------

#include "types.h"

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

/**
 * Declare the possible directions of the robot
 */
typedef enum
{
    STOP, FORWARD, BACKWARD, TANK_TURN,

} groundrobot_direction_t;

/**
 * Create a container for the robot actions
 */
typedef struct
{
        void (*forward)(sint32 angle, uint32 speed);
        void (*backward)(sint32 angle, uint32 speed);
        void (*tankturn)(sint32 angle, uint32 speed);
        void (*stop)(void);
}groundrobot_actions_t;

#endif /* GROUNDROBOT_GROUNDROBOT_H_ */

/**
 * @}
 */
