/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef __MICROPY_INCLUDED_PY_STREAM_H__
#define __MICROPY_INCLUDED_PY_STREAM_H__

#include "py/obj.h"

MP_DECLARE_CONST_FUN_OBJ(mp_stream_read_obj);
MP_DECLARE_CONST_FUN_OBJ(mp_stream_readinto_obj);
MP_DECLARE_CONST_FUN_OBJ(mp_stream_readall_obj);
MP_DECLARE_CONST_FUN_OBJ(mp_stream_unbuffered_readline_obj);
MP_DECLARE_CONST_FUN_OBJ(mp_stream_unbuffered_readlines_obj);
MP_DECLARE_CONST_FUN_OBJ(mp_stream_write_obj);
MP_DECLARE_CONST_FUN_OBJ(mp_stream_seek_obj);
MP_DECLARE_CONST_FUN_OBJ(mp_stream_tell_obj);

// these are for mp_get_stream_raise and can be or'd together
#define MP_STREAM_OP_READ (1)
#define MP_STREAM_OP_WRITE (2)
#define MP_STREAM_OP_IOCTL (4)

const mp_stream_p_t *mp_get_stream_raise(mp_obj_t self_in, int flags);

// Iterator which uses mp_stream_unbuffered_readline_obj
mp_obj_t mp_stream_unbuffered_iter(mp_obj_t self);

mp_obj_t mp_stream_write(mp_obj_t self_in, const void *buf, size_t len);

#if MICROPY_STREAMS_NON_BLOCK
// TODO: This is POSIX-specific (but then POSIX is the only real thing,
// and anything else just emulates it, right?)
#define mp_is_nonblocking_error(errno) ((errno) == EAGAIN || (errno) == EWOULDBLOCK)
#else
#define mp_is_nonblocking_error(errno) (0)
#endif

#endif // __MICROPY_INCLUDED_PY_STREAM_H__
void mp_stream_write_adaptor(void *self, const char *buf, size_t len);
