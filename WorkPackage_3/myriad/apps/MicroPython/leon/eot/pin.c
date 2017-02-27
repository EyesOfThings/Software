/**
 * @file      pin.c
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 24 May 2016
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include "pin.h"

// 2: Global variables
// ----------------------------------------------------------------------------

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//                             Micro python Bindings

/**
 * Store a pin object for every gpio
 */
STATIC const eot_pin_obj_t eot_pin_obj_table[] = { { { &eot_pin_type }, 0, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 1, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 2, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 3, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 4, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 5, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 6, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 7, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 8, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 9, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 10, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 11, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 12, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 13, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 14, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 15, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 16, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 17, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 18, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 19, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 20, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 21, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 22, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 23, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 24, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 25, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 26, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 27, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 28, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 29, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 30, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 31, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 32, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 33, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 34, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 35, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 36, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 37, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 38, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 39, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 40, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 41, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 42, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 43, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 44, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 45, MP_QSTR_MOTOR_DIR0, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 46, MP_QSTR_MOTOR_DIR1, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 47, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 48, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 49, MP_QSTR_MOTOR_PWM0, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 50, MP_QSTR_MOTOR_PWM1, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 51, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 52, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 53, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 54, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 55, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 56, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 57, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 58, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 59, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 60, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 61, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 62, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 63, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 64, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 65, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 66, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 67, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 68, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 69, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 70, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 71, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 72, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 73, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 74, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 75, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 76, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 77, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 78, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 79, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 80, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 81, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 82, MP_QSTR_MOTOR_BRAKE0, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 83, MP_QSTR_NONE, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, 84, MP_QSTR_MOTOR_BRAKE1, IN, MODE_7, true, false },    //
    { { &eot_pin_type }, -1, MP_QSTR_NONE, IN, MODE_7, true, false },    //

        };

/**
 * Get the pin object searching by id
 * @param id:int    The id of the pin (GPIO on the myriad)
 * @return :obj     A pointer to the pin object
 */
const eot_pin_obj_t *get_pin_by_id(int id)
{
    const eot_pin_obj_t *pin;
    bool found_pin = false;

    // Search through the pins
    for (int idx = 0; idx < PIN_COUNT; idx++)
    {
        // Test if the current pin is the one we are looking for
        if (eot_pin_obj_table[idx].id == id)
        {
            // Store a reference to the pin
            pin = &eot_pin_obj_table[idx];

            // Mark the pin as found
            found_pin = true;
        }
    }

    // Test if the pin was not found
    if (!found_pin)
    {
        // Raise an exception
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "Invalid pin id"));
    }

    return pin;
}

/**
 * Set the pin to high
 * @param self_in:obj   The pin object being processed
 * @return  obj         Empty object
 */
STATIC mp_obj_t eot_pin_high(mp_obj_t self_in)
{
    eot_pin_obj_t *self = self_in;

    // Test if the pin has a valid id
    if (self->id > -1)
    {
        // Test if debug is enabled
        if (self->debug)
        {
            printf("Setting pin:%d to high\n", self->id);
        }

        // Set the pin to high
        DrvGpioSetPinHi(self->id);
    }

    return mp_const_none;
}

// Declare the function
STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_pin_high_obj, eot_pin_high);

/**
 * Set the pin to low
 * @param self_in:obj   The object being processed
 * @return :obj         Empty object
 */
STATIC mp_obj_t eot_pin_low(mp_obj_t self_in)
{
    eot_pin_obj_t *self = self_in;

    // Test if the pin has a valid id
    if (self->id > -1)
    {
        // Test if debug is enabled
        if (self->debug)
        {
            printf("Setting pin:%d to low\n", self->id);
        }

        // Set the pin to low
        DrvGpioSetPinLo(self->id);
    }

    return mp_const_none;
}

// Declare the function
STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_pin_low_obj, eot_pin_low);

/**
 * Set the pin to the specified state. Arduino compatible function
 * @param self_in:obj   The pin object to process
 * @param state:int     State to set on the pin. 1 - high, 0 - low
 * @return :obj         Empty object
 */
STATIC mp_obj_t eot_pin_digitalWrite(mp_obj_t self_in, mp_obj_t state)
{
    eot_pin_obj_t *self = self_in;
    bool int_state = mp_obj_get_int(state);

    // test if the pin has a valid id
    if (self->id > -1)
    {
        // Test if debug is enabled
        if (self->debug)
        {
            printf("Setting pin:%d to %d\n", self->id, int_state);
        }

        // Set the pin as output
        pinMode(self->id, OUTPUT);

        // Set the pin to the desired state
        digitalWrite(self->id, int_state);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_pin_digitalWrite_obj, eot_pin_digitalWrite);

/**
 * Read the binary value of the pin. Arduino compatible function
 * @param self_in:obj   The pin object to process
 * @return  :int        State of the pin. 1 - high, 0 - low
 */
STATIC mp_obj_t eot_pin_digitalRead(mp_obj_t self_in)
{
    mp_obj_t pin_value = mp_const_none;
    eot_pin_obj_t *self = self_in;

    // test if the pin has a valid id
    if (self->id > -1)
    {
        // Test if debug is enabled
        if (self->debug)
        {
            printf("Reading state of pin:%d\n", self->id);
        }

        // Set the pin as input
        pinMode(self->id, INPUT);

        // Get the pin state
        pin_value = MP_OBJ_NEW_SMALL_INT(digitalRead(self->id));
    }

    return pin_value;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_pin_digitalRead_obj, eot_pin_digitalRead);

/**
 * Set the pin as PWM and set the specified duty cycle. Arduino compatible function.
 * @param self_in:obj   The pin object to process
 * @param value:int     Number in t e range [0, 255] inclusive to set the pwm duty cycle. 0 corresponds to 0%, 255 corresponds to 100%
 * @return obj          Empty object
 */
STATIC mp_obj_t eot_pin_analogWrite(mp_obj_t self_in, mp_obj_t value)
{
    eot_pin_obj_t *self = self_in;
    u32 int_value = mp_obj_get_int(value);

    // test if the pin has a valid id
    if (self->id > -1)
    {
        // Test if debug is enabled
        if (self->debug)
        {
            printf("Setting pin:%d to %ld\n", self->id, int_value);
        }

        // Set the pin as output
        pinMode(self->id, OUTPUT);

        // Set the pin to the pwm value
        analogWrite(self->id, int_value);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_pin_analogWrite_obj, eot_pin_analogWrite);

/**
 * Print the object
 * @param print:obj_p   Pointer to the print stream
 * @param self_in:obj   Current object being processed
 * @param kind:obj      Kind of message being processed
 */
STATIC void eot_pin_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    eot_pin_obj_t *self = self_in;

    // Print the pin data
    mp_printf(print, "Pin(id = %d, direction = %s)", self->id, self->direction == IN ? "IN" : "OUT");
}

/**
 * Setup the GPIO on the myriad.
 * @param type:obj      The type of the object being created
 * @param n_args:int    Number of arguments passed to the constructor
 * @param n_kw:int      Number of keywords passed to the constructor
 * @param args:obj_p    Arguments passed to the constructor
 * @return obj  The requested pin object
 */
STATIC mp_obj_t eot_pin_obj_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
    // Check that the number of passed arguments
    mp_arg_check_num(n_args, n_kw, 2, 2, false);

    // Get the pin object
    eot_pin_obj_t *pin = get_pin_by_id(mp_obj_get_int(args[0]));

    // Set the direction of the pin
    pin->mode = MODE_7;

    // Set the direction of the pin
    pin->direction = mp_obj_get_int(args[1]);

    // Setup the pin
    DrvGpioSetMode(pin->id, pin->direction | pin->mode);

    // Return the pin object
    return (mp_obj_t) pin;
}

/**
 *  Create the table to store the object function, attribute and constant names
 *  */
STATIC const mp_map_elem_t eot_pin_locals_dict_table[] = {

// instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_high), (mp_obj_t) &eot_pin_high_obj },    // Set the pin to high
    { MP_OBJ_NEW_QSTR(MP_QSTR_low), (mp_obj_t) &eot_pin_low_obj },    // Set the pin to low
    { MP_OBJ_NEW_QSTR(MP_QSTR_digitalWrite), (mp_obj_t) &eot_pin_digitalWrite_obj },    // Set the digital write function
    { MP_OBJ_NEW_QSTR(MP_QSTR_analogWrite), (mp_obj_t) &eot_pin_analogWrite_obj },    // Set the analog write function
    { MP_OBJ_NEW_QSTR(MP_QSTR_digitalRead), (mp_obj_t) &eot_pin_digitalRead_obj },    // Set the digital read function

    // Class constants
    { MP_OBJ_NEW_QSTR(MP_QSTR_IN), MP_OBJ_NEW_SMALL_INT(IN) },    // Set the IN constant
    { MP_OBJ_NEW_QSTR(MP_QSTR_OUT), MP_OBJ_NEW_SMALL_INT(OUT) },    // Set the OUT constant
        };

/**
 * Create the dictionary
 */
STATIC MP_DEFINE_CONST_DICT(eot_pin_locals_dict, eot_pin_locals_dict_table);

/**
 * Create the bindings to the pin
 */
const mp_obj_type_t eot_pin_type = { { &mp_type_type },                //
    .name = MP_QSTR_Pin,                // Set the name of the object (class)
    .print = eot_pin_obj_print,         // Set the funciton to print the object
    .make_new = eot_pin_obj_make_new,    // Set the function to create the object
    .locals_dict = (mp_obj_t) &eot_pin_locals_dict,    // Set the table for the object functions, attributes and constants
        };
