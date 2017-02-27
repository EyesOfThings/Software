/**
 * @file      ground_robot.c
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 31 May 2016
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include "ground_robot.h"

// 2: Global variables
// ----------------------------------------------------------------------------

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

/**
 * Store the possible types of robots
 */
STATIC eot_groundrobot_obj_t robots[] = { { { &eot_groundrobot_type }, Cherokey4WD, MP_QSTR_Cherokey4WD,
    &Cherokey4WD_actions },    //
    { { &eot_groundrobot_type }, SinoningSteering, MP_QSTR_SinoningSteering, &SinoningSteering_actions },    //
    { { &eot_groundrobot_type }, Unknown, MP_QSTR_Unknown },    //
        };

/**
 * Find the robot object by name
 * @param name:qstr     Name of the robot to get
 * @return :obj         The robot object
 */
STATIC eot_groundrobot_obj_t *find_robot_by_name(qstr name)
{
    // Initialize to an unknown robot
    eot_groundrobot_obj_t *robot = &robots[ROBOT_COUNT - 1];

    for (int idx = 0; idx < ROBOT_COUNT; idx++)
    {
        // Test if the current robot is the one we are looking for
        if (strcmp(qstr_str(name), qstr_str(robots[idx].name)) == 0)
        {
            // Get a reference to the robot object
            robot = &robots[idx];
        }
    }

    return robot;
}

/**
 * Convert the pin number on the motor control header to a gpio
 * @param pin:int   The pin number on the Motor Control header
 * @return :int     The GPIO number associated to the pin
 */
STATIC int pin2gpio(int pin)
{
    int gpio = -1;

    // Map the pin on the header to he gpio
    switch (pin)
    {
        case 1:
            gpio = 82;
            break;

        case 2:
            gpio = 84;
            break;

        case 3:
            gpio = 49;
            break;

        case 4:
            gpio = 50;
            break;

        case 5:
            gpio = 45;
            break;

        case 6:
            gpio = 46;
            break;

        default:
            gpio = -1;
            break;
    }

    return gpio;
}

/**
 * Helper function to setup the robot GPIOS and initialization.
 * @param robot:obj_p       Object being set-up
 * @param n_args:int        Number of arguments passed to the constructor
 * @param pos_args:obj_p    Arguments passed by position
 * @param kw_args:obj_p     Keywords passed to the constructor
 * @return :obj     The initialized robot object
 */
STATIC mp_obj_t eot_groundrobot_init_helper(eot_groundrobot_obj_t *robot, mp_uint_t n_args, const mp_obj_t *pos_args,
        mp_map_t *kw_args)
{
    int status = 1;

    static const mp_arg_t allowed_args[] = { { MP_QSTR_pin_m1_en, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
        { MP_QSTR_pin_m1_pwm, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
        { MP_QSTR_pin_m2_en, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
        { MP_QSTR_pin_m2_pwm, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
        { MP_QSTR_pin_state_forward, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
        { MP_QSTR_pin_direction, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
        { MP_QSTR_pin_direction_inv, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
        { MP_QSTR_pin_speed, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
        { MP_QSTR_pin_angle, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 0 } },    //
            };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Test if the robot is the cherokey
    if (robot->type == Cherokey4WD)
    {
        // Test if the pin numbers are valid
        if (pin2gpio(args[0].u_int) > 0 && pin2gpio(args[1].u_int) > 0 && pin2gpio(args[2].u_int) > 0
                && pin2gpio(args[3].u_int) > 0)
        {
            // Initialize the robot
            Cherokey4WDSetup(pin2gpio(args[0].u_int), pin2gpio(args[1].u_int), pin2gpio(args[2].u_int),
                    pin2gpio(args[3].u_int), args[4].u_int);

            // Mark the status as success
            status = 0;
        }
    }
    else if (robot->type == SinoningSteering)
    {
        // Test if the pin numbers are valid
        if (pin2gpio(args[5].u_int) > 0 && pin2gpio(args[6].u_int) > 0 && pin2gpio(args[7].u_int) > 0)
        {
            // Initialize the robot
            SinoningSteeringSetup(pin2gpio(args[5].u_int), pin2gpio(args[6].u_int), pin2gpio(args[7].u_int),
                    pin2gpio(args[8].u_int), args[4].u_int);

            // Mark the status as success
            status = 0;
        }
    }

    // Test if there was any error on the argument parsing
    if (status)
    {
        // Raise an exception
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "Invalid arguments"));
    }

    return mp_const_none;
}

/**
 * Moves the robot forwards
 * @param self_in:obj   The robot object being processed
 * @param angle:uint        Angle of the displacement
 * @param speed:uint        Speed of the robot
 * @return :obj             Empty object
 */
STATIC mp_obj_t eot_groundrobot_forward(mp_obj_t self_in, mp_obj_t angle, mp_obj_t speed)
{
    eot_groundrobot_obj_t *robot = self_in;

    // Move the robot forward
    robot->actions->forward(mp_obj_get_int(angle), mp_obj_get_int(speed));

    return mp_const_none;
}

// Declare the function
STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_groundrobot_forward_obj, eot_groundrobot_forward);

/**
 * Moves the robot backwards
 * @param self_in:obj       The robot object being processed
 * @param angle:uint        Angle of the displacement
 * @param speed:uint        Speed of the robot
 * @return :obj             Empty object
 */
STATIC mp_obj_t eot_groundrobot_backward(mp_obj_t self_in, mp_obj_t angle, mp_obj_t speed)
{
    eot_groundrobot_obj_t *robot = self_in;

    // Move the robot forward
    robot->actions->backward(mp_obj_get_int(angle), mp_obj_get_int(speed));

    return mp_const_none;
}

// Declare the function
STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_groundrobot_backward_obj, eot_groundrobot_backward);

/**
 * Moves the robot in a tank turn if available
 * @param self_in:obj       The robot object being processed
 * @param angle:uint        Angle of the displacement
 * @param speed:uint        Speed of the robot
 * @return :obj             Empty object
 */
STATIC mp_obj_t eot_groundrobot_tankturn(mp_obj_t self_in, mp_obj_t angle, mp_obj_t speed)
{
    eot_groundrobot_obj_t *robot = self_in;

    // Move the robot forward
    robot->actions->tankturn(mp_obj_get_int(angle), mp_obj_get_int(speed));

    return mp_const_none;
}

// Declare the function
STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_groundrobot_tankturn_obj, eot_groundrobot_tankturn);

/**
 * Stop the robot
 * @param self_in:obj   The robot object being processed
 * @return
 */
STATIC mp_obj_t eot_groundrobot_stop(mp_obj_t self_in)
{
    eot_groundrobot_obj_t *robot = self_in;

    // Move the robot forward
    robot->actions->stop();

    return mp_const_none;
}

// Declare the function
STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_groundrobot_stop_obj, eot_groundrobot_stop);

/**
 * Print the object
 * @param print:obj_p   Pointer to the print stream
 * @param self_in:obj   Current object being processed
 * @param kind:obj      Kind of message being processed
 */
STATIC void eot_groundrobot_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    eot_groundrobot_obj_t *robot = self_in;

    // Print the robot data
    mp_printf(print, "GroundRobot(type = %q )", robot->name);
}

/**
 * Create the robot object
 * @param type:obj      The type of the object being created
 * @param n_args:int    Number of arguments passed to the constructor
 * @param n_kw:int      Number of keywords passed to the constructor
 * @param args:obj_p    Arguments passed to the constructor
 * @return obj  The created robot object
 * @return
 */
STATIC mp_obj_t eot_groundrobot_obj_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw,
        const mp_obj_t *args)
{
    // Check that the number of passed arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // Get the Groundrobot object
    eot_groundrobot_obj_t *robot = find_robot_by_name(mp_obj_str_get_qstr(args[0]));

    // Test for the proper arguments
    if (n_args > 1 || n_kw > 0)
    {
        // Initialize the robot
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        eot_groundrobot_init_helper(robot, n_args - 1, args + 1, &kw_args);
    }

    // Return the pin object
    return (mp_obj_t) robot;
}

/**
 *  Create the table to store the object function, attribute and constant names
 *  */
STATIC const mp_map_elem_t eot_groundrobot_locals_dict_table[] = {

// instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_forward), (mp_obj_t) &eot_groundrobot_forward_obj },    // Set the robot to move forward
    { MP_OBJ_NEW_QSTR(MP_QSTR_backward), (mp_obj_t) &eot_groundrobot_backward_obj },    // Set the robot to move backward
    { MP_OBJ_NEW_QSTR(MP_QSTR_tankturn), (mp_obj_t) &eot_groundrobot_tankturn_obj },    // Set the robot to tank turn
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t) &eot_groundrobot_stop_obj },    // Set the robot to stop
        };

/**
 * Create the dictionary
 */
STATIC MP_DEFINE_CONST_DICT( eot_groundrobot_locals_dict, eot_groundrobot_locals_dict_table);

/**
 * Create the bindings to the micropython object
 */
const mp_obj_type_t eot_groundrobot_type = { { &mp_type_type },                //
    .name = MP_QSTR_GroundRobot,                // Set the name of the object (class)
    .print = eot_groundrobot_obj_print,         // Set the funciton to print the object
    .make_new = eot_groundrobot_obj_make_new,    // Set the function to create the object
    .locals_dict = (mp_obj_t) &eot_groundrobot_locals_dict,    // Set the table for the object functions, attributes and constants
        };

