/**
 * @file      pin.h
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 24 May 2016
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include "py/nlr.h"
#include "py/runtime.h"
#include <DrvGpio.h>
#include <stdio.h>
#include "boards/Boards.h"

// 2: Global variables
// ----------------------------------------------------------------------------

/**
 * Set the count of pins on the myriad
 */
#define PIN_COUNT   85

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

/**
 * Create an enumeration for the pin directions
 */
typedef enum
{
    IN = D_GPIO_DIR_IN,     // Set the pin as input
    OUT = D_GPIO_DIR_OUT    // Set the pin as output
} eot_pin_direction_t;


/**
 * Create an enumeration for the pin modes
 */
typedef enum
{
    MODE_1 = D_GPIO_MODE_1,    // GPIO mode from the myriad
    MODE_2 = D_GPIO_MODE_2,    // GPIO mode from the myriad
    MODE_3 = D_GPIO_MODE_3,    // GPIO mode from the myriad
    MODE_4 = D_GPIO_MODE_4,    // GPIO mode from the myriad
    MODE_5 = D_GPIO_MODE_5,    // GPIO mode from the myriad
    MODE_6 = D_GPIO_MODE_6,    // GPIO mode from the myriad
    MODE_7 = D_GPIO_MODE_7,    // GPIO mode from the myriad
} eot_pin_gpio_mode_t;

/**
 * Create the pin object
 */
typedef struct
{
        const mp_obj_base_t base;       // Object type
        const int id;                   // Id of the pin
        const qstr name;                // Name of the pin
        eot_pin_direction_t direction;  // Direction of the pin (IN or OUT)
        eot_pin_gpio_mode_t mode;       // Pin mode (mode on the myriad)
        bool digital;                   // Flag that indicates the pin is being used as digital (no pwm)
        bool debug;                     // Flag to control debug messages
} eot_pin_obj_t;

/**
 * Create a reference to the pin type
 */
extern const mp_obj_type_t eot_pin_type;
