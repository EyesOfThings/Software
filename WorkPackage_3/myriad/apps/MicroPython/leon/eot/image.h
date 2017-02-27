/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "ccv.h"
#include "../condStates.h"

/**
 * Create the image object
 */
typedef struct
{
        mp_obj_base_t base;
        ccv_dense_matrix_t * _cobj;
} eot_image_obj_t;

void *py_image_cobj_ptr(mp_obj_t img_obj);


/**
 * Create a reference to the image type
 */

//extern unsigned char image_show_buffer[480*256];
extern unsigned char *image_to_show;
extern int image_size_in_bytes_to_show;

/*extern unsigned char** image_to_show_buffer;
extern int size_image_to_show;
extern int images_saved;
extern int image_size_buffer[100];*/

//extern unsigned char image_to_show[368640];
//extern int image_size_in_bytes_to_show;

extern const mp_obj_type_t eot_image_type;
