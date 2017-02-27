/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   camera.h
 * Author: chema
 *
 * Created on June 3, 2016, 9:57 AM
 */

#include "py/nlr.h"
#include "py/runtime.h"

#ifndef CAMERA_H
#define CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    
    mp_obj_base_t base;
} eot_camera_obj_t;

extern const mp_obj_type_t eot_camera_type;

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_H */

