/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "camera.h"
#include <camera.h> //Camera lib
#include "image.h"
#include <stdio.h>

static eot_camera_obj_t* mp_EoT_Camera;

// Python syntax:   mCamera = eot.Camera()
STATIC mp_obj_t eot_camera_obj_make_new (const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    if ((mp_obj_t)(mp_EoT_Camera) != MP_OBJ_NULL) {
        return (mp_obj_t) mp_EoT_Camera;
    }
    
    mp_EoT_Camera = m_new_obj(eot_camera_obj_t);
    mp_EoT_Camera->base.type = &eot_camera_type;
    printf ("Camera initialized\n");
    return (mp_obj_t) mp_EoT_Camera;
}

// Python syntax:   mSnapshot = mCamera.snapshot()
static mp_obj_t eot_camera_snapshot() {
    //setImgSize(640,480);
    take_snapshot();
    int cols, rows;
    //int cols=480, rows=256;
    getImgSize(&cols,&rows);
    eot_image_obj_t* snapshot = m_new_obj(eot_image_obj_t);
    snapshot->base.type = &eot_image_type;
    snapshot->_cobj = ccv_dense_matrix_new (rows, cols, CCV_8U | CCV_C1, 0, 0);
    int i, j;
     for (i = 0; i < (*snapshot->_cobj).rows; i++)
            for (j = 0; j < (*snapshot->_cobj).cols; j++)
                    (*snapshot->_cobj).data.u8[i * (*snapshot->_cobj).step + j] = last_frame_buffer[i * (*snapshot->_cobj).step + j].Y+0x80;

    return (mp_obj_t) snapshot;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_camera_snapshot_obj, eot_camera_snapshot);

// Python syntax:   mSnapshot = mCamera.setImgSize(rows,cols)
static mp_obj_t eot_camera_setImgSize(mp_obj_t img_obj, mp_obj_t rows, mp_obj_t cols) {

    int _rows = mp_obj_get_int(rows);
    int _cols = mp_obj_get_int(cols);

    setImgSize(_rows,_cols);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_camera_setImgSize_obj, eot_camera_setImgSize);

STATIC const mp_map_elem_t eot_camera_locals_dict_table[] = {
    
        { MP_OBJ_NEW_QSTR(MP_QSTR_snapshot), (mp_obj_t) &eot_camera_snapshot_obj },    // Get a snapshot
        { MP_OBJ_NEW_QSTR(MP_QSTR_setImgSize), (mp_obj_t) &eot_camera_setImgSize_obj },    // Set the camera image size

};

STATIC MP_DEFINE_CONST_DICT (eot_camera_locals_dict, eot_camera_locals_dict_table);

const mp_obj_type_t eot_camera_type = {
    { &mp_type_type },
    .name = MP_QSTR_Camera,
    .make_new = eot_camera_obj_make_new,
    .locals_dict = (mp_obj_t) &eot_camera_locals_dict,
};
