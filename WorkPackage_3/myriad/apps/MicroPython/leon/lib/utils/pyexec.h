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
#ifndef __MICROPY_INCLUDED_LIB_UTILS_PYEXEC_H__
#define __MICROPY_INCLUDED_LIB_UTILS_PYEXEC_H__

typedef enum {
    PYEXEC_MODE_RAW_REPL,
    PYEXEC_MODE_FRIENDLY_REPL,
} pyexec_mode_kind_t;

extern pyexec_mode_kind_t pyexec_mode_kind;

#define PYEXEC_FORCED_EXIT (0x100)
#define PYEXEC_SWITCH_MODE (0x200)

int pyexec_raw_repl(void);
int pyexec_friendly_repl(void);
int pyexec_file(const char *filename);
int pyexec_frozen_module(const char *name);
void pyexec_event_repl_init(void);
int pyexec_event_repl_process_char(int c);

MP_DECLARE_CONST_FUN_OBJ(pyb_set_repl_info_obj);

#endif // __MICROPY_INCLUDED_LIB_UTILS_PYEXEC_H__
