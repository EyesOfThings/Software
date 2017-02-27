/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SDIO.h
 * Author: chema
 *
 * Created on 21 de julio de 2016, 10:03
 */

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/runtime0.h"
#include <SDCardIO.h>
#include "py/objstr.h"
#include <stdlib.h>
#include <string.h>


#ifndef SDIO_H
#define SDIO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    mp_obj_base_t base;
    SDCardFile* _cobj;
} eot_FileIO_obj_t;

extern const mp_obj_type_t eot_FileIO_type;


#ifdef __cplusplus
}
#endif

#endif /* SDIO_H */

