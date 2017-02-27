/**
 * @file      eotmod.c
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 25 May 2016
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include "py/nlr.h"
#include "eot/pin.h"
#include "eot/image.h"
#include "eot/camera.h"
#include "eot/ground_robot.h"
//#include "eot/FileIO.h"
#include "eot/SDIO.h"

// 2: Global variables
// ----------------------------------------------------------------------------

// 3: Class / Functions definitions
// ----------------------------------------------------------------------------

/**
 * Create the table to associate the strings to objects
 */
STATIC const mp_map_elem_t eot_module_globals_table[] =
{
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_eot) },    // Map the name of the module
    { MP_OBJ_NEW_QSTR(MP_QSTR_Pin), (mp_obj_t) &eot_pin_type },             // Map the pin object
    { MP_OBJ_NEW_QSTR(MP_QSTR_GroundRobot), (mp_obj_t) &eot_groundrobot_type }, // Map the GroundRobot from the MotorControlAPI object
    { MP_OBJ_NEW_QSTR(MP_QSTR_Image), (mp_obj_t) &eot_image_type },         // Map the image object
    { MP_OBJ_NEW_QSTR(MP_QSTR_Camera), (mp_obj_t) &eot_camera_type},        // Map the camera object
    { MP_OBJ_NEW_QSTR(MP_QSTR_SDIO), (mp_obj_t) &eot_FileIO_type},        // Map the SDIO object
   //{ MP_OBJ_NEW_QSTR(MP_QSTR_FileIO), (mp_obj_t) &eot_FileIO_type},        // Map the FileIO object
};

/**
 * Define the dictionary entry for the object table
 */
STATIC MP_DEFINE_CONST_DICT(eot_module_globals, eot_module_globals_table);

/**
 * Create the module object
 */
const mp_obj_module_t eot_module =
{
    .base = { &mp_type_module },
    .name = MP_QSTR_eot,
    .globals = (mp_obj_dict_t*) &eot_module_globals,
};

