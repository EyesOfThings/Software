#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>

#include <mv_types.h>

#include "rtems_config.h"
#include "app_config.h"

#include <SDCardIO.h>

#include <rtems/cpuuse.h>


//INCLUDES MICROPYTHON
#include <stdint.h>

#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "lib/utils/pyexec.h"

char buff_micropython[1024];

//FUNCTIONS MICROPYTHON 1
void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    //output
    memset(buff_micropython, 0x00, sizeof(buff_micropython));
    strcpy(buff_micropython,">");
    
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
    if (lex == NULL) {
        printf("MemoryError: lexer could not allocate memory\n");
        return;
    }

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        //printf("1111111111111 New instruction 1111111111111\n");
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, true);
        
        mp_call_function_0(module_fun); //RUN PYTHON CODE
        
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
    
    //output
    if(strlen(buff_micropython)>1)
        printf("%s",buff_micropython);
}

static char *stack_top;
static char heap[10240]; //MEMMORY FOR PYTHON

// ----------------------------------------------------------------------------
// 6: Functions Implementation
// ----------------------------------------------------------------------------

void POSIX_Init(void *args)  
{

    initClocksAndMemory();

    //MICROPYTHON
    int stack_dummy;
    stack_top = (char*) &stack_dummy;

    //printf("Point 1\n");
    
    gc_init(heap, heap + sizeof (heap));
    mp_init();
    
    //do_str("a=5;", MP_PARSE_SINGLE_INPUT);
    //do_str("a", MP_PARSE_SINGLE_INPUT);
    do_str("b=5;", MP_PARSE_SINGLE_INPUT);
    do_str("a=b*6", MP_PARSE_SINGLE_INPUT);
    do_str("a", MP_PARSE_SINGLE_INPUT);
    do_str("list1 = [1, 2, 3, 4, 5 ]",MP_PARSE_SINGLE_INPUT);
    do_str("list1[2] = 2001",MP_PARSE_SINGLE_INPUT);
    do_str("list1",MP_PARSE_SINGLE_INPUT);
    do_str("len(list1)",MP_PARSE_SINGLE_INPUT);
    do_str("list1.append(33)",MP_PARSE_SINGLE_INPUT);
    do_str("len(list1)",MP_PARSE_SINGLE_INPUT);
    do_str("list1",MP_PARSE_SINGLE_INPUT);
    
    do_str("print('hello world!')", MP_PARSE_SINGLE_INPUT);
    
    do_str("for i in range(10):\r\n  print(i)", MP_PARSE_FILE_INPUT);
    
    do_str("print('hello world!', list(x+1 for x in range(10)), end='eol\\n')", MP_PARSE_SINGLE_INPUT);
    
    
    /*
    printf("Point 2\n");
    
    
    
    printf("Point 3\n");
    
    do_str("for i in range(10):\r\n  print(i)", MP_PARSE_FILE_INPUT);
    do_str("a=100", MP_PARSE_FILE_INPUT);
    int a = 1;
    while (a < 10) {
        do_str("a=100+a", MP_PARSE_FILE_INPUT);
        do_str("a", MP_PARSE_FILE_INPUT);
        a++;
    }
    
    printf("Point 4\n");
    */
    
    mp_deinit();
    
    //printf("Point 5\n");
    
    //END MICROPYTHON
    return 0;

    exit(0);
}

void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info();
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    return NULL;
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(uint n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val) {
}

void NORETURN __fatal_error(const char *msg) {
    while (1);
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif


void Fatal_extension(
	Internal_errors_Source	the_source,
	bool 					is_internal,
	uint32_t				the_error
)
{
	switch(the_source) {
	case RTEMS_FATAL_SOURCE_EXIT:
		if (the_error)
			printk("Exited with error code %d\n", the_error);
		break;
	case RTEMS_FATAL_SOURCE_ASSERT:
		printk("%s : %d in %s \n",
				((rtems_assert_context *)the_error)->file,
               	((rtems_assert_context *)the_error)->line,
               	((rtems_assert_context *)the_error)->function);
		break;
	case RTEMS_FATAL_SOURCE_EXCEPTION:
		rtems_exception_frame_print((const rtems_exception_frame *) the_error);
		break;
	default:
		printk("\nSource %d Internal %d Error %d 0x%X:\n", 
			the_source, is_internal, the_error, the_error);
		break;
	}
}