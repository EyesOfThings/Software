/**
 * @file      Cherokey4WDBoards.h
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 20 Oct 2015
 * Author: dexmont
 *
 * @defgroup GroupName Group title
 * @{
 * @brief     Group briefing
 *
 * Detailed description
 */

#ifndef _MOTIONCONTROLBOARDS_H_
#define _MOTIONCONTROLBOARDS_H_

// 1: Includes
// ----------------------------------------------------------------------------
#define BOARD_DEBUG

#define MYRIAD

// Call the specific header for the board
#ifdef ARDUINO
#include "Arduino.h"
#endif

#ifdef RASPBERRY_PI
#include "RaspberryPi.h"
#endif

#ifdef MYRIAD
#include "Myriad/Myriad.h"
#include "Myriad/Servo.h"
#endif

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------



// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

#endif /* _MOTIONCONTROLBOARDS_H_ */

/**
 * @}
 */
