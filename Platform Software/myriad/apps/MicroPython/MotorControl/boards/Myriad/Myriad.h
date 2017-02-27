/**
 * @file      Cherokey4WDMyriad.h
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 20 Oct 2015
 * Author: dexmont
 *
 * @defgroup Cherokey4WDMyriad Cherokey4WD from Myriad
 * @{
 * @brief     Controls the Cherokey4WD from the Myriad processor
 *
 * Defines the required functions to control the Myriad pins
 */

#ifndef _CHEROKEY4WDMYRIAD_H_
#define _CHEROKEY4WDMYRIAD_H_

#ifdef MYRIAD

// 1: Includes
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <mv_types.h>
#include <DrvGpio.h>
#include <DrvTimer.h>
#include <registersMyriad.h>
#include <DrvRegUtils.h>
#include <types.h>

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// Define states of the pins
#define LOW     0
#define HIGH    1

// Define the modes of the pins
#define OUTPUT  D_GPIO_DIR_OUT | D_GPIO_MODE_7
#define INPUT   D_GPIO_DIR_IN | D_GPIO_MODE_7

// Define the status
#define SUCCESS     0
#define ERROR       1

// Set the maximum number of allowed pwm pins (13 hardware + software, used for allocating space for the counters)
#define MYRIAD_PWM_MAX_PIN_COUNT    15

// Set the length of the pwm count for a full cycle
#define MYRIAD_HWPWM_DEFAULT_TICK_COUNT  12000

// Set the parameters for the software pwm
#define MYRIAD_SWPWM_IRQ_INTERVAL         10     // Time between interrupt calls in microseconds
#define MYRIAD_SWPWM_IRQ_PRIORITY         7      // Interrupt priority
#define MYRIAD_SWPWM_MAX_PIN_COUNT        (MYRIAD_PWM_MAX_PIN_COUNT - 13)       // Set the maximum amount of pins which can be software pwm
#define MYRIAD_SWPWM_DEFAULT_TICK_COUNT              3000

// Create a struct to store the sw pwm counters
typedef struct
{
    uint32 pin_id;
    uint8 state;
    uint64 time_high;
    uint64 time_low;
    uint64 expire_time;
}Myriad_SWPWM_counter_data;

typedef struct
{
    /**
     * Store the pin id
     */
    uint8 pin_id;

    /**
     * For hardware pwm pins, store the block, otherwise not used
     */
    sint8 block;

    /**
     * Store the number of ticks required to complete a pwm cycle
     */
    uint64 pwm_count;
}Myriad_PWM_data;

// 3: Used namespaces
// ----------------------------------------------------------------------------

// 4: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 5: Class / Functions declarations
// ----------------------------------------------------------------------------

/**
 * Sets the duty cycle for the specified PWM pin
 * @param pin_id
 * @param duty_cyle
 */
void setPWMDutyCycle(uint32 pin_id, uint32 duty_cyle);

/**
 * Sets pin on high for the specified number of ticks
 * @param pin_id
 * @param pwm_count
 */
void setPWMTimeHigh(uint32 pin_id, uint32 pwm_count);

/**
 * Sets tick count for the period of the pwm for the specified pin
 * @param pin_id
 * @param period
 */
void setPWMPeriod(uint32 pin_id, uint64 period);

/**
 * Gets the PWM block associated to the pin, if the pin as no PWM block returns -1
 * @param pin_id
 * @return
 */
sint8 getPWMBlock(uint32 pin_id);

/**
 * Sets the pin as software pwm
 * @param pin_id
 * @param pwm_count The number of ticks required for a pwm cycle
 */
void setPinAsSWPWM(uint32 pin_id, uint32 pwm_count);

/**
 * Interrupt handler for software pwm
 * @param timerNumber
 * @param param2
 * @return
 */
uint32 MyriadSWPWMIRQHandle(uint32 timerNumber, uint32 param2);

/**
 * Get the index of the pin if any
 * @param pin_id
 * @return -1 if the pin is not found, otherwise the index of the pin data
 */
sint8 getPinIndex(uint32 pin_id);

/**
 * Get the mode to which the pin has to be set to enable hardware pwm
 * @param pin_id    GPIO to test
 * @return  Pin mode to use or -1 if not supported
 */
sint32 getPinModePWM(uint32 pin_id);

/**
 * Register the pin as hw pwm
 * @param pin_id    GPIO to register
 * @return  SUCCESS(0) or ERROR(>0)
 */
uint32 registerPinPWM(uint32 pin_id);

/**
 * Register the pin as software pwm
 * @param pin_id    GPIO to register
 * @param pwm_count Tick count in microseconds for completing a cycle
 * @return  SUCCESS(0) or ERROR(>0)
 */
uint32 registerPinSWPWM(uint32 pin_id, uint32 pwm_count);

/**
 * Get the pin digital state
 * @param pin_id    GPIO id to read
 * @return State of the pin (HIGH or LOW)
 */
uint32 digitalRead(uint32 pin_id);

/**
 * Set the pin to the specified state
 * @param pin_id    GPIO id to set
 * @param state     State of the pin (HIGH or LOW)
 */
void digitalWrite(uint32 pin_id, uint32 state);

/**
 * Set the pin to the pwm value,
 * @param pin_id    GPIO to set
 * @param value     The PWM duty cycle. The values must be between 0 and 255
 */
void analogWrite(uint32 pin_id, uint32 value);

/**
 * Set the pin to the specified mode.
 * @param pin_id    GPIO to set
 * @param mode Mode to set on the pin. Values are INPUT, OUTPUT
 */
void pinMode(uint32 pin_id, uint32 mode);

#endif

#endif /* _CHEROKEY4WDMYRIAD_H_ */

/**
 * @}
 */
