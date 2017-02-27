/**
 * @file      image.c
 */
#include "image.h"
#include <stdio.h>
#include <SDCardIO.h>
#include "jpeg_codec.h"
#include <math.h>
#include <stdlib.h>

unsigned char *image_to_show;
//colorYCbCr *last_frame_buffer3;//[2920*1080];
//colorYCbCr last_frame_buffer3[491520];

//colorYCbCr last_frame_buffer2[262144];

int image_size_in_bytes_to_show;


/*unsigned char** image_to_show_buffer;
int size_image_to_show=368640;
int images_saved=-1;
int image_size_buffer[100];*/
/*
 * Create the bindings to the image
 */
// First some helper macros and functions...
#define IM_IS_GrayScale(img) \
    ({ CCV_GET_CHANNEL((img)->type) == CCV_C1; })

#define IM_GET_GrayScale_PIXEL(img, row, col) \
    ({ ((img)->data.u8)[(row*(img)->step)+col]; })

#define IM_SET_GrayScale_PIXEL(img, row, col, p) \
    ({ ((img)->data.u8)[(row*(img)->step)+col] = p; })

#define IM_ROW_INSIDE_LIMITS(img, row) \
    ({ (0<=row)&&(row<(img)->rows); })

#define IM_COL_INSIDE_LIMITS(img, col) \
    ({ (0<=col)&&(col<(img)->cols); })

#define ccv_get_dense_matrix_cell_value_by2(x, row, col, ch) \
        (((x->type) & CCV_32S) ? mp_obj_new_int((x)->data.i32[((row) * (x)->cols + (col)) * CCV_GET_CHANNEL(x->type) + (ch)]) : \
        (((x->type) & CCV_32F) ? mp_obj_new_float((x)->data.f32[((row) * (x)->cols + (col)) * CCV_GET_CHANNEL(x->type) + (ch)]) : \
        (((x->type) & CCV_64S) ? mp_obj_new_int((x)->data.i64[((row) * (x)->cols + (col)) * CCV_GET_CHANNEL(x->type) + (ch)]) : \
        (((x->type) & CCV_64F) ? mp_obj_new_float((float)(x)->data.f64[((row) * (x)->cols + (col)) * CCV_GET_CHANNEL(x->type) + (ch)]) : \
        mp_obj_new_int((x)->data.u8[(row) * (x)->step + (col) * CCV_GET_CHANNEL(x->type) + (ch)])))))

#define ccv_set_value2(x, row, col, value, ch) switch (CCV_GET_DATA_TYPE((x->type))) { \
        case CCV_32S: (x)->data.i32[((row) * (x)->cols + (col)) * CCV_GET_CHANNEL(x->type) + (ch)] = mp_obj_get_int(value); break; \
        case CCV_32F: (x)->data.f32[((row) * (x)->cols + (col)) * CCV_GET_CHANNEL(x->type) + (ch)] = mp_obj_get_float(value); break; \
        case CCV_64S: (x)->data.i64[((row) * (x)->cols + (col)) * CCV_GET_CHANNEL(x->type) + (ch)] = mp_obj_get_int(value); break; \
        case CCV_64F: (x)->data.f64[((row) * (x)->cols + (col)) * CCV_GET_CHANNEL(x->type) + (ch)] = mp_obj_get_float(value); break; \
        default: (x)->data.u8[(row) * (x)->step + (col) * CCV_GET_CHANNEL(x->type) + (ch)] = mp_obj_get_int(value); }

/**
 * Create a dense matrix with rows, cols, and type.
 * \note Python syntax: imageResult = Image(rows, cols, type)
 * \note Python example: imageResult = Image(5, 4, Image.CCV_8U | Image.CCV_C1)
 * \note Python syntax from a file: imageResult = Image(path, flag)
 * \note Python example: imageResult = Image("/mnt/sdcard/lena.png", CCV_IO_RGB_COLOR);
 * @param rows:int Rows of the matrix.
 * @param cols:int Columns of the matrix.
 * @param type:int Matrix supports 4 data types - CCV_8U, CCV_32S, CCV_64S, CCV_32F, CCV_64F and up to 255 channels. e.g. CCV_32F | 31 will create a matrix with float (32-bit float point) data type with 31 channels (the default type for ccv_hog).
 * @param flag:int CCV_IO_GRAY, convert to grayscale image. CCV_IO_RGB_COLOR, convert to color image.
 * @return **image** The newly created matrix object.
 */
STATIC mp_obj_t eot_image_obj_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // Check that the number of passed arguments
    mp_arg_check_num(n_args, n_kw, 2, 3, false);
    eot_image_obj_t* o = m_new_obj(eot_image_obj_t);

    // Create the image object from file
    if (n_args == 2) {
        int _type = mp_obj_get_int(args[1]);

        SDCardMount();
        ccv_dense_matrix_t* _dst_img = 0;
        o->base.type = &eot_image_type;
        o->_cobj = 0;

        ccv_read(mp_obj_str_get_str(args[0]), (&o->_cobj), _type | CCV_IO_ANY_FILE);
        SDCardUnmount();
    }
    // Create a empty image object from file
    // arg[0] rows param
    // arg[1] cols param
    else if (n_args == 3) {
        int _type = mp_obj_get_int(args[2]);
        o->base.type = &eot_image_type;
        o->_cobj = ccv_dense_matrix_new(mp_obj_get_int(args[0]), mp_obj_get_int(args[1]), _type, 0, 0);
        ccv_zero(o->_cobj);
    }

    // Return the image object
    return (mp_obj_t) o;
}

/**
 * Print the information about a image. Cols and rows number, channel type and data type.
 * \note Python syntax: print(img_obj)
 * @param img_obj:image The object pointer. The input image.
 */

STATIC void eot_image_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    eot_image_obj_t *self = self_in;
    // Print the image data
    mp_printf(print, "Image cols %d rows %d channel_type %d data_type %d \n", self->_cobj->cols, self->_cobj->rows, CCV_GET_CHANNEL(self->_cobj->type), CCV_GET_DATA_TYPE(self->_cobj->type));
}

void *py_image_cobj_ptr(mp_obj_t img_obj) {
    return &((eot_image_obj_t *) img_obj)->_cobj;
}

/**
 * Get image width
 * \note Python syntax: width = img_obj.width()
 * @param img_obj:image The object pointer. The input image.
 * @return **int** Width
 */
static mp_obj_t eot_image_width(mp_obj_t img_obj) {
    ccv_dense_matrix_t **arg_img = py_image_cobj_ptr(img_obj);
    return mp_obj_new_int((*arg_img)->cols);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_image_width_obj, eot_image_width);

/**
 * Get image height
 * \note Python syntax: height = img_obj.height()
 * @param img_obj:image The object pointer. The input image.
 * @return **int** Height
 */
static mp_obj_t eot_image_height(mp_obj_t img_obj) {
    ccv_dense_matrix_t **arg_img = py_image_cobj_ptr(img_obj);
    return mp_obj_new_int((*arg_img)->rows);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_image_height_obj, eot_image_height);

/**
 * Get a pixel
 * \note Python syntax: pixel = img_obj.set_pixel(row, col)
 * \note Python syntax: pixelR, pixelG, pixelB = img_obj.get_pixel(row,col) for a color image
 * @param img_obj:image The object pointer. The input image.
 * @param row:int Pixel row
 * @param col:int Pixel col
 * @return If img_obj is CCV_C1, the function returns a value **int**. Otherwise, the function returns a tuple with as many parameters as channels.
 */

static mp_obj_t eot_image_get_pixel(mp_obj_t img_obj, mp_obj_t row, mp_obj_t col) {
    ccv_dense_matrix_t **arg_img = py_image_cobj_ptr(img_obj);
    int _row = mp_obj_get_int(row);
    int _col = mp_obj_get_int(col);

    if ((!IM_ROW_INSIDE_LIMITS(*arg_img, _row)) || (!IM_COL_INSIDE_LIMITS(*arg_img, _col)))
        return mp_const_none;

    int n_ch = 0;

    if ((n_ch = CCV_GET_CHANNEL((*arg_img)->type)) == 1)
        return ccv_get_dense_matrix_cell_value_by2((*arg_img), _row, _col, 0);
    else {
        mp_obj_t pixel_tuple[n_ch];
        int i = 0;
        for (i; i < n_ch; i++)
            pixel_tuple[i] = ccv_get_dense_matrix_cell_value_by2((*arg_img), _row, _col, i);
        return mp_obj_new_tuple(n_ch, pixel_tuple);
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_get_pixel_obj, eot_image_get_pixel);

/**
 * Set a pixel
 * \note Python syntax: img_obj.set_pixel(row, col, pixel)
 * \note Python syntax: img_obj.set_pixel(row,col,(r,g,b)) for a color image
 * @param img_obj:image The object pointer. The input image.
 * @param row:int Pixel row
 * @param col:int Pixel col
 * @param pixel:int This value can be a tuple
 */

static mp_obj_t eot_image_set_pixel(uint n_args, const mp_obj_t *args) {
    ccv_dense_matrix_t **arg_img = py_image_cobj_ptr(args[0]);
    int row = mp_obj_get_int(args[1]);
    int col = mp_obj_get_int(args[2]);

    if ((!IM_ROW_INSIDE_LIMITS(*arg_img, row)) || (!IM_COL_INSIDE_LIMITS(*arg_img, col)))
        return mp_const_none;

    int n_ch = 0;
    int i = 0;
    if ((n_ch = CCV_GET_CHANNEL((*arg_img)->type)) == 1) {
        ccv_set_value2((*arg_img), row, col, args[3], 0);
    } else {
        mp_obj_t *arg_color;
        mp_obj_get_array_fixed_n(args[3], n_ch, &arg_color);
        for (i = 0; i < n_ch; i++) {
            ccv_set_value2((*arg_img), row, col, arg_color[i], i);
        }
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_set_pixel_obj, 4, 4, eot_image_set_pixel);

mp_obj_t py_image_create_object_from_image(ccv_dense_matrix_t *src_img) {
    // create destination image object...
    eot_image_obj_t* dst_obj = m_new_obj(eot_image_obj_t);
    dst_obj->base.type = &eot_image_type;
    dst_obj->_cobj = ccv_dense_matrix_new(src_img->rows, src_img->cols, src_img->type, 0, 0); //note this will allocate memory for the image

    // copy image data...
    memcpy((dst_obj->_cobj)->data.u8, src_img->data.u8, src_img->rows * src_img->step);

    return dst_obj;
}

/**
 * Copy a image
 * \note Python syntax: imageResult = img_obj.copy()
 * @param img_obj:image The object pointer. The input image.
 * @return **image** A copy of img_obj
 */
static mp_obj_t eot_image_copy(mp_obj_t img_obj) {
    ccv_dense_matrix_t **src_img = py_image_cobj_ptr(img_obj);

    // create and return destination image object...
    return (py_image_create_object_from_image(*src_img));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_image_copy_obj, eot_image_copy);

/**
 * Compute image with Sobel operator.
 * \note Python syntax: imageResult = img_obj.sobel(type, dx, dy)
 * \note Python example: imageResult = img_obj.sobel(0, 1, 3)
 * @param img_obj:image The object pointer. The input image.
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param dx:int The window size of Sobel operator on x-axis, specially optimized for 1, 3
 * @param dy:int The window size of Sobel operator on y-axis, specially optimized for 1, 3
 * @return **image** The resulting image
 */
static mp_obj_t eot_image_sobel(uint n_args, const mp_obj_t *args) {
    ccv_dense_matrix_t **src_img = py_image_cobj_ptr(args[0]);
    int _type = mp_obj_get_int(args[1]);
    int _szx = mp_obj_get_int(args[2]);
    int _szy = mp_obj_get_int(args[3]);

    // perform operation...
    ccv_dense_matrix_t *_dst_img = 0;
    ccv_sobel(*src_img, &_dst_img, _type, _szx, _szy);

    // create destination imageobject...
    //mp_obj_t o eot_image_obj_t?
    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return dst_obj;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_sobel_obj, 4, 4, eot_image_sobel);

// eot_image_write wrapper
// Python syntax: Img1.save(path)

/**
 * Save a image into SDCard
 * \note Python syntax: img_obj.save(path)
 * \note Python example: img_obj.save("/mnt/sdcard/image.png")
 * @param img_obj:image The object pointer. The input image.
 * @param path:string Image path
 */
static mp_obj_t eot_image_save(mp_obj_t img_obj, mp_obj_t path) {
    SDCardMount();
    ccv_dense_matrix_t **src_img = py_image_cobj_ptr(img_obj);

    ccv_write(*src_img, mp_obj_str_get_str(path), 0, CCV_IO_PNG_FILE, 0);
    SDCardUnmount();
    return (mp_const_none);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_image_save_obj, eot_image_save);

/**
 * Show a Image
 * \note Python syntax: img_obj.show()
 * \warning Works with CCV_C1 and CCV_C3 image type
 * @param img_obj:image The input matrix.
 */
static mp_obj_t eot_image_show(mp_obj_t img_obj)
{
    //free (last_frame_buffer3);
    //int i=0, j=0;
    ccv_dense_matrix_t **img_src = py_image_cobj_ptr(img_obj);

    convertDenseMatrix2Jpeg(*img_src);

    //save images to send
    /*images_saved++;
    image_to_show_buffer[images_saved] = NULL;
    while (image_to_show_buffer[images_saved] == NULL) {
        //image_to_show_buffer[received_packet] = (unsigned char *) malloc(payload_max);
        image_to_show_buffer[images_saved] = malloc(sizeof (unsigned char)*size_image_to_show);
        //printf("Allocating memmory %d \n",actual_packet);
    }
    memcpy(image_to_show_buffer[images_saved], image_to_show, size_image_to_show);
    image_size_buffer[images_saved]=image_size_in_bytes_to_show;*/
    


    // Unlocks Pulga
    pthread_mutex_lock(&mutex);
    state = SHOW_IMAGE;
    pthread_cond_signal(&pulgaTurn);
    pthread_mutex_unlock(&mutex);
    // Waits until the image is sent
    pthread_mutex_lock(&mutex);
    while (state != EXECUTE_PYTHON)
        pthread_cond_wait(&pythonTurn, &mutex);
    pthread_mutex_unlock(&mutex);


    //return mp_obj_new_str("op_show_image", strlen ("op_show_image"), true);
    return (mp_const_none);

}


STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_image_show_obj, eot_image_show);

/**
 * Compute the gradient (angle and magnitude) at each pixel.
 * \note Python syntax = theta, magnitude = img_obj.gradient (ttype, mtype, dx, dy)
 * \note Python example = theta, magnitude = img_obj.gradient(0, 0, 1, 3)
 * @param img_obj:image The object pointer. The input image.
 * @param ttype The type of theta output matrix, if 0, ccv will defaults to CCV_32F. Theta matrix is the output matrix of angle at each pixel.
 * @param mtype The type of magnitude output matrix, if 0, ccv will defaults to CCV_32F. Magnitude matrix is the output matrix of magnitude at each pixel.
 * @param dx The window size of the underlying Sobel operator used on x-axis, specially optimized for 1, 3
 * @param dy The window size of the underlying Sobel operator used on y-axis, specially optimized for 1, 3
 * @return A tuple (theta, magnitude) (**image**, **image**)
 */
static mp_obj_t eot_image_gradient(uint n_args, const mp_obj_t *args) {
    ccv_dense_matrix_t **src_img = py_image_cobj_ptr(args[0]);
    int _ttype = mp_obj_get_int(args[1]);
    int _mtype = mp_obj_get_int(args[2]);
    int _szx = mp_obj_get_int(args[3]);
    int _szy = mp_obj_get_int(args[4]);

    // perform operation...
    ccv_dense_matrix_t *theta = 0, *_dst_img = 0;
    ccv_gradient(*src_img, &theta, _ttype, &_dst_img, _mtype, _szx, _szy);

    // create destination image object...
    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);
    mp_obj_t theta_obj = py_image_create_object_from_image(theta);

    mp_obj_t result_tuple[2];
    result_tuple[0] = theta_obj;
    result_tuple[1] = dst_obj;

    ccv_matrix_free(_dst_img);
    ccv_matrix_free(theta);

    return mp_obj_new_tuple(2, result_tuple);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_gradient_obj, 5, 5, eot_image_gradient);

/**
 * Flip the matrix by x-axis, y-axis or both.
 * \note Python syntax: imgResult = img_obj.flip(btype, flag)
 * \note Python example: imgResult = img_obj.flip(0, Image.CCV_FLIP_X)
 * @param img_obj:image The object pointer. The input image.
 * @param btype:int The type of output matrix, if 0, ccv will use the sample type as the input matrix.
 * @param flag:int Image.CCV_FLIP_X - flip around x-axis, Image.CCV_FLIP_Y - flip around y-axis.
 * @return **image** The resulting image.
 */
static mp_obj_t eot_image_flip(mp_obj_t img_obj, mp_obj_t btype, mp_obj_t flag) {

    ccv_dense_matrix_t ** src_img = py_image_cobj_ptr(img_obj);
    int _btype = mp_obj_get_int(btype);
    int _flag = mp_obj_get_int(flag);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_flip(*src_img, &_dst_img, _btype, _flag);

    // create destination image object...
    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return dst_obj;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_flip_obj, eot_image_flip);

/**
 * Using Gaussian blur on a given matrix. It implements a O(n * sqrt(m)) algorithm, n is the size of input matrix, m is the size of Gaussian filtering kernel.
 * \note Python syntax: imgResult = img_obj.blur(btype, sigma)
 * \note Python example: imgResult = img_obj.blur(0, 2)
 * @param img_obj:image The object pointer. The input image.
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param sigma:float The sigma factor in Gaussian filtering kernel.
 * @return **image** The resulting image
 */
static mp_obj_t eot_image_blur(mp_obj_t img_obj, mp_obj_t btype, mp_obj_t sigma) {

    ccv_dense_matrix_t** src_img = py_image_cobj_ptr(img_obj);

    int _type = mp_obj_get_int(btype);
    float _sigma = mp_obj_get_float(sigma);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_blur(*src_img, &_dst_img, _type, _sigma);

    // create destination image object...
    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return dst_obj;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_blur_obj, eot_image_blur);

/**
 * Histogram-of-Oriented-Gradients implementation, specifically, it implements the HOG described in *Object Detection with Discriminatively Trained Part-Based Models, Pedro F. Felzenszwalb, Ross B. Girshick, David McAllester and Deva Ramanan*.
 * \note Python syntax: imgResult = img_obj.hog(b_type, sbin, size)
 * \note Python example: imgResult = img_obj.hog(0, 31, 8)
 * @param img_obj:image The object pointer. The input image.
 * @param b_type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param sbin:int The number of bins for orientation (default to 9, thus, for **b**, it will have 9 * 2 + 9 + 4 = 31 channels).
 * @param size:int The window size for HOG (default to 8)
 * @return **image** Histogram of oriented gradients
 */
static mp_obj_t eot_image_hog(uint n_args, const mp_obj_t *args) {

    ccv_dense_matrix_t** src_img = py_image_cobj_ptr(args[0]);
    ccv_dense_matrix_t* _dst_img = 0;

    int _b_type = mp_obj_get_int(args[1]);
    int _sbin = mp_obj_get_int(args[2]);
    int _size = mp_obj_get_int(args[3]);

    ccv_hog(*src_img, &_dst_img, _b_type, _sbin, _size);

    // create destination image object...
    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return dst_obj;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_hog_obj, 4, 4, eot_image_hog);



/**
 * Canny edge detector implementation. For performance reason, this is a clean-up reimplementation of OpenCV's Canny edge detector, it has very similar performance characteristic as the OpenCV one.
 * \note Python syntax: imgResult = img_obj.canny(type, size, low_thresh, high_thresh)
 * \note Python example: imgResult = img_obj.canny(0, 3, 3, 3*36)
 * \warning Canny edge detector only works with CCV_8U or CCV_32S dense matrix type and img_obj channel must be CCV_C1.

 * @param img_obj:image The object pointer. The input image.
 * @param type:int The type of output matrix, if 0, ccv will create a CCV_8U | CCV_C1 matrix.
 * @param size:int The underlying Sobel filter size.
 * @param low_thresh:float The low threshold that makes the point interesting.
 * @param high_thresh:float The high threshold that makes the point acceptable.
 * @return **image** The resulting image
 */
static mp_obj_t eot_image_canny(mp_uint_t n_args, const mp_obj_t *args) {

    //Image result
    ccv_dense_matrix_t **src_img = py_image_cobj_ptr(args[0]);
    ccv_dense_matrix_t* dst_img = 0;
    int _type = mp_obj_get_int(args[1]);
    int _size = mp_obj_get_int(args[2]);
    float _low_thresh = mp_obj_get_float(args[3]);
    float _high_thresh = mp_obj_get_float(args[4]);
    ccv_canny(*src_img, &dst_img, _type, _size, _low_thresh, _high_thresh);
    mp_obj_t dst_obj = py_image_create_object_from_image(dst_img);
    ccv_matrix_free(dst_img);
    return dst_obj;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_canny_obj, 5, 5, eot_image_canny);

/**
 * OTSU implementation.
 * \note Python syntax: outvarRes, thresholdRes = img_obj.otsu(outvar, range)
 * \note Python example: outvarRes, thresholdRes = img_obj.otsu(20,150)
 * \warning Only works with CCV_8U or CCV_32S dense matrix data type
 * @param img_obj:image The object pointer. The input image.
 * @param outvar:float The inter-class variance.
 * @param range:int The maximum range of data in the input matrix.
 * @return A tuple (outvarRes, thresholdRes) (**float**, **int**), where thresholdRes is the threshold, inclusively. e.g. 5 means 0~5 is in the background, and 6~255 is in the foreground.
 */
static mp_obj_t eot_image_otsu(mp_obj_t img_obj, mp_obj_t outvar, mp_obj_t range) {

    ccv_dense_matrix_t** src_img = py_image_cobj_ptr(img_obj);

    double _outvar = (double) mp_obj_get_float(outvar);
    int _range = mp_obj_get_int(range);
    float _outvar2 = (float) _outvar;
    int _threshold = ccv_otsu(*src_img, &_outvar, _range);

    mp_obj_t result_tuple[2];
    result_tuple[0] = mp_obj_new_float(_outvar);
    result_tuple[1] = mp_obj_new_int(_threshold);
    return mp_obj_new_tuple(2, result_tuple);

}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_otsu_obj, eot_image_otsu);

/**
 * Downsample a given matrix to exactly half size with a Gaussian filter. The half size is approximated by floor(rows * 0.5) x floor(cols * 0.5).
 * \note Python syntax: imageRes = img_obj.sample_down(type, src_x, src_y)
 * \note Python example: imageRes = img_obj.sample_down(0, 2, 2)
 * \warning src_x and src_y variables must be greater than 0
 * @param img_obj:image The object pointer. The input image.
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param src_x:int Shift the start point by src_x.
 * @param src_y:int Shift the start point by src_y.
 * @result **image** The resulting image
 */
static mp_obj_t eot_image_sample_down(mp_uint_t n_args, const mp_obj_t *args) {

    ccv_dense_matrix_t** src_img = py_image_cobj_ptr(args[0]);
    int _type = mp_obj_get_int(args[1]);
    int _src_x = mp_obj_get_int(args[2]);
    int _src_y = mp_obj_get_int(args[3]);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_sample_down(*src_img, &_dst_img, _type, _src_x, _src_y);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);
    ccv_matrix_free(_dst_img);

    return dst_obj;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_sample_down_obj, 4, 4, eot_image_sample_down);

/**
 * Upsample a given matrix to exactly double size with a Gaussian filter.
 * \note Python syntax: imageRes = img_obj.sample_up(type, src_x, src_y)
 * \note Python example: imageRes = img_obj.sample_up(0, 3, 3)
 * \warning src_x and src_y variables must be greater than 0
 * @param img_obj:image The object pointer. The input image.
 * @param b The output matrix.
 * @param type The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param src_x Shift the start point by src_x.
 * @param src_y Shift the start point by src_y.
 * @result **image** The resulting image
 */
static mp_obj_t eot_image_sample_up(mp_uint_t n_args, const mp_obj_t *args) {
    ccv_dense_matrix_t** src_img = py_image_cobj_ptr(args[0]);
    int _type = mp_obj_get_int(args[1]);
    int _src_x = mp_obj_get_int(args[2]);
    int _src_y = mp_obj_get_int(args[3]);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_sample_up(*src_img, &_dst_img, _type, _src_x, _src_y);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);
    ccv_matrix_free(_dst_img);

    return dst_obj;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_sample_up_obj, 4, 4, eot_image_sample_up);

/**
 * Matrix addition.
 * \note Python syntax: imgResult = img_obj.add(img2, type)
 * \note Python example: imgResult = img_obj.add(img2, 0)
 * \warning rows, cols and channel of img_obj and img2 must be the same.
 * @param img_obj:image The object pointer. The input matrix
 * @param img2:image The input matrix.
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @return **image** The resulting matrix
 */
static mp_obj_t eot_image_add(mp_obj_t img_obj, mp_obj_t img2, mp_obj_t type) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);
    ccv_dense_matrix_t ** _img2 = py_image_cobj_ptr(img2);

    ccv_dense_matrix_t* _dst_img = 0;
    int _type = mp_obj_get_int(type);

    ccv_add(*_img1, *_img2, & _dst_img, _type);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_add_obj, eot_image_add);

/**
 * Normalize a matrix and return the normalize factor.
 * 
 * \note Python syntax: imgResult = img_obj.normalize(btype, flag)
 * \note Python example: imgResult = img_obj.normalize(0, Image.CCV_L1_NORM)
 * \warning img_obj channel must be CCV_C1
 * @param img_obj:image The object pointer. The input matrix.
 * @param btype:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param flag:int Image.CCV_L1_NORM (|dx| + |dy|) or CCV_L2_NORM (sqrt(dx^2 + dy^2)), for L1 or L2 normalization.
 * @return **image** L1 or L2 sum.
 */
static mp_obj_t eot_image_normalize(mp_obj_t img_obj, mp_obj_t btype, mp_obj_t flag) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);

    int btype2 = mp_obj_get_int(btype);
    int flag2 = mp_obj_get_int(flag);

    ccv_dense_matrix_t* _dst_img = 0;

    double _dst_sum = ccv_normalize(*_img1, & _dst_img, (*_img1)->type, flag2);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    float _dst_sum2 = (float) _dst_sum;

    mp_obj_t result_tuple[2];

    result_tuple[0] = mp_obj_new_float(_dst_sum2);
    result_tuple[1] = dst_obj;

    ccv_matrix_free(_dst_img);

    return mp_obj_new_tuple(2, result_tuple);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_normalize_obj, eot_image_normalize);

/**
 * Generate the Summed Area Table
 * \note Python syntax: imgResult = img_obj.sat(type, padding_pattern)
 * \note Python example: imgResult = img_obj.sat(0, Image.CCV_NO_PADDING)
 * @param img_obj:image The object pointer. The input matrix.
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param padding_pattern:int Image.CCV_NO_PADDING - the first row and the first column in the output matrix is the same as the input matrix. Image.CCV_PADDING_ZERO - the first row and the first column in the output matrix is zero, thus, the output matrix size is 1 larger than the input matrix.
 * @return **image** The Summed Area Table
 */
static mp_obj_t eot_image_sat(mp_obj_t img_obj, mp_obj_t type, mp_obj_t padding_pattern) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);

    int type2 = mp_obj_get_int(type);
    int padding_pattern2 = mp_obj_get_int(padding_pattern);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_sat(*_img1, & _dst_img, (*_img1)->type, padding_pattern2);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_sat_obj, eot_image_sat);

/**
 * Return the sum of all elements in the matrix.
 * \note Python syntax: sum = img_obj.sum(flag)
 * \note Python example: sum = img_obj.sum(Image.CCV_UNSIGNED)
 * @param img_obj:image The object pointer. The input matrix
 * @param flag:int Image.CCV_UNSIGNED - compute fabs(x) of the elements first and then sum up. Image.CCV_SIGNED - compute the sum normally.
 * @return **float** The sum of all elements in the matrix
 */
static mp_obj_t eot_image_sum(mp_obj_t img_obj, mp_obj_t flag) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);

    int flag2 = mp_obj_get_int(flag);

    double _dst_sum = ccv_sum(*_img1, flag2);
    float _dst_sum2 = (float) _dst_sum;

    return mp_obj_new_float(_dst_sum2);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_image_sum_obj, eot_image_sum);

/**
 * Return the variance of all elements in the matrix.
 * \note Python syntax: variance = img_obj.variance()
 * @param img_obj:image The object pointer. The input matrix
 * @return **float** Element variance of the input matrix.
 */
static mp_obj_t eot_image_variance(mp_obj_t img_obj) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);


    double _dst_variance = ccv_variance(*_img1);
    float _dst_variance2 = (float) _dst_variance;

    return mp_obj_new_float(_dst_variance2);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_image_variance_obj, eot_image_variance);


//python syntax img3 = img1.multiply(img2, 0)

/**
 * Do element-wise matrix multiplication.
 * \note Python syntax: imgResult = img_obj.multiply(img2, type)
 * \note Python example: imgResult = img_obj.multiply(img2, 0)
 * \warning rows, cols, data type and channel of img_obj and img2 must be the same.
 * @param img_obj:image The object pointer. The input matrix
 * @param img2:image The input matrix.
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @return **image** The resulting matrix.
 */
static mp_obj_t eot_image_multiply(mp_obj_t img_obj, mp_obj_t img2, mp_obj_t type) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);
    ccv_dense_matrix_t ** _img2 = py_image_cobj_ptr(img2);
    int type2 = mp_obj_get_int(type);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_multiply(*_img1, *_img2, & _dst_img, (*_img1)->type); //(ccv_matrix_t**) & _dst_img

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_multiply_obj, eot_image_multiply);

/**
 * Matrix subtraction.
 * \note Python syntax: imgResult = img_obj.subtract(img2, type)
 * \note Python example: imgResult = img_obj.subtract(img2, 0)
 * \warning rows, cols, data type and channel of img_obj and img2 must be the same.
 * @param img_obj:image The object pointer. The input matrix
 * @param img2:image The input matrix.
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @return **image** The resulting matrix.
 */
static mp_obj_t eot_image_subtract(mp_obj_t img_obj, mp_obj_t img2, mp_obj_t type) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);
    ccv_dense_matrix_t ** _img2 = py_image_cobj_ptr(img2);
    int type2 = mp_obj_get_int(type);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_subtract(*_img1, *_img2, & _dst_img, type2);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_subtract_obj, eot_image_subtract);

/**
 * Scale given matrix by factor of **ds**.
 * \note Python syntax: imgResult = img_obj.scale(type, ds)
 * \note Python example: imgResult = img_obj.scale(0, 2)
 * @param img_obj:image The object pointer. The input matrix
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param ds:float The scale factor, `imgResult = img_obj * ds`
 * @return **image** The resulting Matrix
 */
static mp_obj_t eot_image_scale(mp_obj_t img_obj, mp_obj_t type, mp_obj_t ds) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);

    int type2 = mp_obj_get_int(type);
    float ds2 = mp_obj_get_float(ds);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_scale(*_img1, & _dst_img, (*_img1)->type, ds2);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_scale_obj, eot_image_scale);

/**
 * General purpose matrix multiplication.
 *
 * As general as it is, it computes:
 *
 *   alpha * img_obj * img2 + beta * img3
 *
 * whereas img_obj, img2, img3 are matrix, and alpha, beta are scalar.
 *
 * \note Python syntax: imageResult = img_obj.gemm(img2, alpha, img3, beta, transpose)
 * \note Python example: imageResult = img_obj.gemm(img2, 1, 0, 0, 0)
 * \warning This function has a hard dependency on [cblas](http://www.netlib.org/blas/) library.
 * \warning Image type must be Image.CCV_32F or Image.CCV_64F
 * @param img_obj:image The input matrix.
 * @param img2:image The input matrix.
 * @param alpha:float The multiplication factor.
 * @param img3:image The input matrix. It can be 0.
 * @param beta:float The multiplication factor.
 * @param transpose:int Image.CCV_A_TRANSPOSE, Image.CCV_B_TRANSPOSE to indicate if matrix A or B need to be transposed first before multiplication.
 * @return **image** The resulting matrix 
 */
static mp_obj_t eot_image_gemm(uint n_args, const mp_obj_t *args) {

    ccv_dense_matrix_t** src_img_a = py_image_cobj_ptr(args[0]);
    ccv_dense_matrix_t** src_img_b = py_image_cobj_ptr(args[1]);
    
    float alpha = mp_obj_get_float(args[2]);    
    float beta = mp_obj_get_float(args[4]);
    int transpose = mp_obj_get_int(args[5]);
    int type = mp_obj_get_int(args[6]);
    ccv_dense_matrix_t* _dst_img = 0;
    
    if (MP_OBJ_IS_TYPE(args[3], &eot_image_type) == 1) {
        ccv_dense_matrix_t** src_img_c = py_image_cobj_ptr (args[3]);
        ccv_gemm(*src_img_a, *src_img_b, alpha, *src_img_c, beta, transpose, &_dst_img, type);  
    }
    else {
        ccv_dense_matrix_t* src_img_c= 0;
        ccv_gemm(*src_img_a, *src_img_b, alpha, &src_img_c, beta, transpose, &_dst_img, type);

    }
    // create destination image object...
    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return dst_obj;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_gemm_obj, 7, 7, eot_image_gemm);

/**
 * Convert matrix from one color space representation to another.
 * \note Python syntax: imageResult = img_obj.color_transform(type)
 * \note Python example: imageResult = img_obj.color_transform(0)
 * \warning Only works in RGB space (CCV_C3)
 * @param img_obj:image The input matrix.
 * @param type:int The type of output matrix, if 0, ccv will use the sample type as the input matrix.
 * @return **image** The resulting matrix
 */
static mp_obj_t eot_image_color_transform(mp_obj_t img_obj, mp_obj_t type) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);

    int type2 = mp_obj_get_int(type);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_color_transform(*_img1, &_dst_img, type2, CCV_RGB_TO_YUV);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);

}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_image_color_transform_obj, eot_image_color_transform);

/**
 * Manipulate image's saturation.
 * \note Python syntax: imageResult = img_obj.saturation(type, ds)
 * \note Python example: imageResult = img_obj.saturation(0, 3)
 * \warning Only works in RGB space
 * @param img_obj:image The input matrix.
 * @param type:int The type of output matrix, if 0, ccv will use the sample type as the input matrix.
 * @param ds:float The coefficient (0: grayscale, 1: original).
 * @return **image** The resulting matrix
 */
static mp_obj_t eot_image_saturation(mp_obj_t img_obj, mp_obj_t type, mp_obj_t ds) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);

    int type2 = mp_obj_get_int(type);
    float ds2 = mp_obj_get_float(ds);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_saturation(*_img1, &_dst_img, type2, ds2);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);

}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_saturation_obj, eot_image_saturation);

/**
 * Manipulate image's contrast.
 * \note Python syntax: imageResult = img_obj.contrast(type, ds)
 * \note Python example: imageResult = img_obj.contrast(0, 4)
 * @param img_obj:image The input matrix.
 * @param type:int The type of output matrix, if 0, ccv will use the sample type as the input matrix.
 * @param ds:float The coefficient (0: mean image, 1: original).
 * @return **image** The resulting matrix
 */
static mp_obj_t eot_image_contrast(mp_obj_t img_obj, mp_obj_t type, mp_obj_t ds) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(img_obj);

    int type2 = mp_obj_get_int(type);
    float ds2 = mp_obj_get_float(ds);

    ccv_dense_matrix_t* _dst_img = 0;

    ccv_contrast(*_img1, &_dst_img, type2, ds2);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);

}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_image_contrast_obj, eot_image_contrast);

/**
 * Convolve on dense matrix a with dense matrix b.
 * \note Python syntax: imageResult = img_obj.filter(img2, type, padding_pattern)
 * \note Python example: imageResult = img_obj.filter()
 * \warning This function has a soft dependency on [FFTW3](http://fftw.org/). If no FFTW3 exists, ccv will use [KissFFT](http://sourceforge.net/projects/kissfft/) shipped with it. FFTW3 is about 35% faster than KissFFT.
 * @param img_obj:image The input matrix.
 * @param img2:image Dense matrix b.
 * @param type:int The type of output matrix, if 0, ccv will try to match the input matrix for appropriate type.
 * @param padding_pattern:int ccv doesn't support padding pattern for now.
 * @return **image** The resulting image
 */

static mp_obj_t eot_image_filter(uint n_args, const mp_obj_t *args) {
    ccv_dense_matrix_t ** _img1 = py_image_cobj_ptr(args[0]);
    ccv_dense_matrix_t ** _img2 = py_image_cobj_ptr(args[1]);

    int _type = mp_obj_get_int(args[2]);
    int _padding_pattern = mp_obj_get_int(args[3]);
    
    ccv_dense_matrix_t* _dst_img;

    ccv_filter(*_img1, *_img2, &_dst_img, _type, _padding_pattern);

    mp_obj_t dst_obj = py_image_create_object_from_image(_dst_img);

    ccv_matrix_free(_dst_img);

    return (dst_obj);


}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_filter_obj, 4, 4, eot_image_filter);
/**
 * Free the image data
 * \note Python syntax: img_obj.free_image_data()
 * \warning MicroPython does not delete the data stored when reassigned, making several copies, so it is necessary to free it manually. For deleting the image reference is necessary to use the **del** function (img_obj.del()), otherwise, developer could use the img_obj variable, but it will be empty 
 * @param img_obj:image The input matrix.
 */
static mp_obj_t eot_image_free_image_data(mp_obj_t img_obj) {
    ccv_dense_matrix_t ** _img = py_image_cobj_ptr(img_obj);
    ccv_matrix_free((*_img));
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_image_free_image_data_obj, eot_image_free_image_data);

/**
 * Using a BBF classifier cascade to detect objects in a given image. If you have several classifier cascades, it is better to use them in one method call. In this way, ccv will try to optimize the overall performance.
 * \note Python syntax: list_faces = img_obj.detect_face_bbf(cascade_path)
 * \note Python example: list_faces = img_obj.detect_face_bbf("/mnt/sdcard/cascade")
 * @param img_obj:image The input image.
 * @param cascade_path:string An array of classifier cascades.
 * @return **list of lists** The list of list contains the faces coordenates. Example: [ [pos_x_img1, pos_y_img1, width_img1, height_img1, classification_img1], [pos_x_img2, pos_y_img2, width_img2, height_img2, classification_img2] ]
 */

static mp_obj_t eot_image_detect_face_bbf(mp_obj_t img_obj, mp_obj_t cascade_path) {
    SDCardMount();
    ccv_dense_matrix_t** _src_img = py_image_cobj_ptr(img_obj);
    ccv_bbf_classifier_cascade_t* cascade = ccv_bbf_read_classifier_cascade(mp_obj_str_get_str(cascade_path));
    ccv_array_t* seq = ccv_bbf_detect_objects(*_src_img, &cascade, 1, ccv_bbf_default_params);
    int i=0;
    mp_obj_list_t *faces_list = MP_OBJ_TO_PTR(mp_obj_new_list(seq->rnum, NULL));
    for (i=0; i < seq->rnum; i++) {
        ccv_comp_t* comp = (ccv_comp_t*)ccv_array_get(seq, i);
        faces_list->items[i] = MP_OBJ_TO_PTR(mp_obj_new_list(5, NULL));
        ((mp_obj_list_t*) (faces_list->items[i]))->items[0] = mp_obj_new_int(comp->rect.x);
        ((mp_obj_list_t*) (faces_list->items[i]))->items[1] = mp_obj_new_int(comp->rect.y);
        ((mp_obj_list_t*) (faces_list->items[i]))->items[2] = mp_obj_new_int(comp->rect.width);
        ((mp_obj_list_t*) (faces_list->items[i]))->items[3] = mp_obj_new_int(comp->rect.height);
        ((mp_obj_list_t*) (faces_list->items[i]))->items[4] = mp_obj_new_float(comp->classification.confidence);
         free(comp);
    }
    ccv_array_free(seq);
   
    ccv_bbf_classifier_cascade_free(cascade);
    SDCardUnmount();
    //res
    return MP_OBJ_FROM_PTR(faces_list);
   
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_image_detect_face_bbf_obj, eot_image_detect_face_bbf);

/**
 * Draw a rectangle
 * \note Python syntax: img_obj.draw_rectangle(x, y, width, height)
 * \note Python example: img_obj.draw_rectangle([132, 4, 91, 91)
 * @param img_obj:image The input image.
 * @param x:int Coordenate x of the start of the rectangle
 * @param y:int Coordenate y of the start of the rectangle
 * @param width:int Rectangle width
 * @param height:int Rectangle height
 */
static mp_obj_t eot_image_draw_rectangle (mp_uint_t n_args, const mp_obj_t *args)
{
 
    ccv_dense_matrix_t **src_img = py_image_cobj_ptr (args[0]);
 
    //if ((!IM_ROW_INSIDE_LIMITS(*arg_img, _row)) || (!IM_COL_INSIDE_LIMITS(*arg_img, _col)))
    //    return mp_const_none;
 
    int _x = mp_obj_get_int(args[1]);
    int _y = mp_obj_get_int(args[2]);
    int _w = mp_obj_get_int(args[3]);
    int _h = mp_obj_get_int(args[4]);
    int _color = mp_obj_get_int (args[5]);
    int i=0;
    int _cols = (*src_img)->cols;
    int _rows = (*src_img)->rows;
    //printf ("x %d y %d w %d h %d color %d rows %d cols %d \n", _x, _y, _w, _h, _color, _cols, _rows);
 
    for (i; i<_w; i++) {
        (*src_img)->data.u8[(_y*_cols+_x)+i] = _color;
        (*src_img)->data.u8[((_y-1)*_cols+(_x))+i] = _color;
        (*src_img)->data.u8[((1+_y)*_cols+(_x))+i] = _color;
 
        (*src_img)->data.u8[(_y*_cols+_x+(_cols*_h)+i)] = _color;
        (*src_img)->data.u8[((_y-1)*_cols+_x+(_cols*_h)+i)] = _color;
        (*src_img)->data.u8[((_y+1)*_cols+_x+(_cols*_h)+i)] = _color;
    }
 
    for (i=0; i<_h; i++) {
        (*src_img)->data.u8[(_y*_cols+_x)+(_cols*i)] = _color;
        (*src_img)->data.u8[(_y*_cols+_x-1)+(_cols*i)] = _color;
        (*src_img)->data.u8[(_y*_cols+_x+1)+(_cols*i)] = _color;
 
        (*src_img)->data.u8[(_y*_cols+_x+_w)+(_cols*i)] = _color;
        (*src_img)->data.u8[(_y*_cols+_x+_w-1)+(_cols*i)] = _color;
        (*src_img)->data.u8[(_y*_cols+_x+_w+1)+(_cols*i)] = _color;
    }
 
    return mp_const_none;
 
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(eot_image_draw_rectangle_obj, 6, 6, eot_image_draw_rectangle);


STATIC const mp_map_elem_t eot_image_locals_dict_table[] = {

    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_width), (mp_obj_t) & eot_image_width_obj}, // Get width
    { MP_OBJ_NEW_QSTR(MP_QSTR_height), (mp_obj_t) & eot_image_height_obj}, // Get height
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel), (mp_obj_t) & eot_image_set_pixel_obj}, //Set a pixel
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel), (mp_obj_t) & eot_image_get_pixel_obj}, //Get a pixel
    { MP_OBJ_NEW_QSTR(MP_QSTR_copy), (mp_obj_t) & eot_image_copy_obj}, //Copy a image object
    { MP_OBJ_NEW_QSTR(MP_QSTR_sobel), (mp_obj_t) & eot_image_sobel_obj}, //Sobel operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_show), (mp_obj_t) & eot_image_show_obj}, //Show a image
    { MP_OBJ_NEW_QSTR(MP_QSTR_save), (mp_obj_t) & eot_image_save_obj}, //Save a image
    { MP_OBJ_NEW_QSTR(MP_QSTR_gradient), (mp_obj_t) & eot_image_gradient_obj}, //Gradient operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_flip), (mp_obj_t) & eot_image_flip_obj}, //Flip operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_blur), (mp_obj_t) & eot_image_blur_obj}, //blur operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_canny), (mp_obj_t) & eot_image_canny_obj}, //canny operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_hog), (mp_obj_t) & eot_image_hog_obj}, //hog operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_otsu), (mp_obj_t) & eot_image_otsu_obj}, //otsu operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_sample_down), (mp_obj_t) & eot_image_sample_down_obj}, //sample_down operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_sample_up), (mp_obj_t) & eot_image_sample_up_obj}, //sample_up operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_color_transform), (mp_obj_t) & eot_image_color_transform_obj}, //color_transform operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_saturation), (mp_obj_t) & eot_image_saturation_obj}, //saturation operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_contrast), (mp_obj_t) & eot_image_contrast_obj}, //contrast operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_filter), (mp_obj_t) & eot_image_filter_obj}, //contrast operation
    { MP_OBJ_NEW_QSTR(MP_QSTR_add), (mp_obj_t) & eot_image_add_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_normalize), (mp_obj_t) & eot_image_normalize_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_sat), (mp_obj_t) & eot_image_sat_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_sum), (mp_obj_t) & eot_image_sum_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_variance), (mp_obj_t) & eot_image_variance_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_multiply), (mp_obj_t) & eot_image_multiply_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_subtract), (mp_obj_t) & eot_image_subtract_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_scale), (mp_obj_t) & eot_image_scale_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_gemm), (mp_obj_t) & eot_image_gemm_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_free_image_data), (mp_obj_t) & eot_image_free_image_data_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_detect_face_bbf), (mp_obj_t) & eot_image_detect_face_bbf_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_draw_rectangle), (mp_obj_t) & eot_image_draw_rectangle_obj},


    //Class constants     
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_8U), MP_OBJ_NEW_SMALL_INT(0x01000)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_32S), MP_OBJ_NEW_SMALL_INT(0x02000)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_32F), MP_OBJ_NEW_SMALL_INT(0x04000)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_64S), MP_OBJ_NEW_SMALL_INT(0x08000)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_64F), MP_OBJ_NEW_SMALL_INT(0x10000)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_C1), MP_OBJ_NEW_SMALL_INT(0x001)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_C2), MP_OBJ_NEW_SMALL_INT(0x002)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_C3), MP_OBJ_NEW_SMALL_INT(0x003)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_C4), MP_OBJ_NEW_SMALL_INT(0x004)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_IO_GRAY), MP_OBJ_NEW_SMALL_INT(0x100)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_IO_RGB_COLOR), MP_OBJ_NEW_SMALL_INT(0x300)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_INTER_AREA), MP_OBJ_NEW_SMALL_INT(0x01)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_INTER_CUBIC), MP_OBJ_NEW_SMALL_INT(0x04)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_RGB_TO_YUV), MP_OBJ_NEW_SMALL_INT(0x01)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_L1_NORM), MP_OBJ_NEW_SMALL_INT(0x01)}, // Set the CCV_L1_NORM constant
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_L2_NORM), MP_OBJ_NEW_SMALL_INT(0x02)}, // Set the CCV_L2_NORM constant
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_NO_PADDING), MP_OBJ_NEW_SMALL_INT(0x00)}, // Set the CCV_L1_NORM constant
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_PADDING_ZERO), MP_OBJ_NEW_SMALL_INT(0x01)}, // Set the CCV_L2_NORM constant
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_UNSIGNED), MP_OBJ_NEW_SMALL_INT(0x01)}, // Set the CCV_L2_NORM constant
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_SIGNED), MP_OBJ_NEW_SMALL_INT(0x00)}, // Set the CCV_L2_NORM constant
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_FLIP_X), MP_OBJ_NEW_SMALL_INT(0x01)}, // Set the CV_FLIP_X constant
    { MP_OBJ_NEW_QSTR(MP_QSTR_CCV_FLIP_Y), MP_OBJ_NEW_SMALL_INT(0x02)}// Set the CV_FLIP_Y constant

};

/*
 * Create the dictionary
 */
STATIC MP_DEFINE_CONST_DICT(eot_image_locals_dict, eot_image_locals_dict_table);

const mp_obj_type_t eot_image_type = {
    { &mp_type_type}, //
    .name = MP_QSTR_Image, // Set the name of the object (class)
    .print = eot_image_obj_print, // Set the funciton to print the object
    .make_new = eot_image_obj_make_new, // Set the function to create the object
    .locals_dict = (mp_obj_t) & eot_image_locals_dict, // Set the table for the object functions, attributes and constants
};
