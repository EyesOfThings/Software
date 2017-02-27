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
#ifndef __MICROPY_INCLUDED_PY_EMITGLUE_H__
#define __MICROPY_INCLUDED_PY_EMITGLUE_H__

#include "py/obj.h"

// These variables and functions glue the code emitters to the runtime.

typedef enum {
    MP_CODE_UNUSED,
    MP_CODE_RESERVED,
    MP_CODE_BYTECODE,
    MP_CODE_NATIVE_PY,
    MP_CODE_NATIVE_VIPER,
    MP_CODE_NATIVE_ASM,
} mp_raw_code_kind_t;

typedef struct _mp_raw_code_t mp_raw_code_t;

mp_raw_code_t *mp_emit_glue_new_raw_code(void);

void mp_emit_glue_assign_bytecode(mp_raw_code_t *rc, const byte *code, mp_uint_t len,
    const mp_uint_t *const_table,
    #if MICROPY_PERSISTENT_CODE_SAVE
    uint16_t n_obj, uint16_t n_raw_code,
    #endif
    mp_uint_t scope_flags);
void mp_emit_glue_assign_native(mp_raw_code_t *rc, mp_raw_code_kind_t kind, void *fun_data, mp_uint_t fun_len, const mp_uint_t *const_table, mp_uint_t n_pos_args, mp_uint_t scope_flags, mp_uint_t type_sig);

mp_obj_t mp_make_function_from_raw_code(mp_raw_code_t *rc, mp_obj_t def_args, mp_obj_t def_kw_args);
mp_obj_t mp_make_closure_from_raw_code(mp_raw_code_t *rc, mp_uint_t n_closed_over, const mp_obj_t *args);

#if MICROPY_PERSISTENT_CODE_LOAD
typedef struct _mp_reader_t {
    void *data;
    mp_uint_t (*read_byte)(void *data);
    void (*close)(void *data);
} mp_reader_t;

mp_raw_code_t *mp_raw_code_load(mp_reader_t *reader);
mp_raw_code_t *mp_raw_code_load_mem(const byte *buf, size_t len);
mp_raw_code_t *mp_raw_code_load_file(const char *filename);
#endif

#if MICROPY_PERSISTENT_CODE_SAVE
void mp_raw_code_save(mp_raw_code_t *rc, mp_print_t *print);
void mp_raw_code_save_file(mp_raw_code_t *rc, const char *filename);
#endif

#endif // __MICROPY_INCLUDED_PY_EMITGLUE_H__
