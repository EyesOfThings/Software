/* Structures for JSON parsing using only fixed-extent memory
 *
 * This implementation is based on MircoJSON 1.3, which can be found unmodified
 * at: http://www.catb.org/esr/microjson/
 * This is the original copyright notice:
 * This file is Copyright (c) 2014 by Eric S. Raymond.
 * BSD terms apply: see the file COPYING for details.
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    t_integer,
    t_uinteger,
    t_float,
    t_string,
    t_boolean,
    t_character,
    t_object,
    t_structobject,
    t_array,
    t_ignore
} json_type;

struct json_enum_t {
    char* name;
    int value;
};

struct json_array_t {
    json_type element_type;
    union {
        struct {
            const struct json_attr_t* subtype;
            char* base;
            size_t stride;
        } objects;
        struct {
            char** ptrs;
            char* store;
            int storeLen;
        } strings;
        struct {
            int* store;
        } integers;
        struct {
            unsigned int* store;
        } uintegers;
        struct {
            float* store;
        } reals;
        struct {
            bool* store;
        } booleans;
    } arr;
    int* count;
    int maxLen;
};

struct json_attr_t {
    char* attribute;
    json_type type;
    union {
        int* integer;
        unsigned int* uinteger;
        float* real;
        char* string;
        bool* boolean;
        char* character;
        struct json_array_t array;
        size_t offset;
    } addr;
    union {
        int integer;
        unsigned int uinteger;
        float real;
        bool boolean;
        char character;
    } defaultValue;
    size_t len;
    const struct json_enum_t* map;
    bool noDefault;
};

#ifdef __cplusplus
extern "C" {
#endif

int json_read_object(const char* json,
                     const struct json_attr_t* objectAttributes,
                     const char** end);

int json_read_array(const char* json,
                    const struct json_array_t* arrayAttributes,
                    const char** end);

const char* json_error_string(int);

#ifdef __cplusplus
}
#endif

/*
 * Use the following macros to declare template initializers for structobject
 * arrays.  Writing the equivalents out by hand is error-prone.
 *
 * STRUCTOBJECT takes a structure name s, and a field name f in s.
 *
 * STRUCTARRAY takes the name of a structure array, a pointer to a an
 * initializer defining the subobject type, and the address of an integer to
 * store the length in.
 */
#define STRUCTOBJECT(structName, fieldName) \
    .addr.offset = offsetof(structName, fieldName)

#define STRUCTARRAY(arrayPtr, elementType, length) \
    .addr.array.element_type = t_structobject, \
    .addr.array.arr.objects.subtype = elementType, \
    .addr.array.arr.objects.base = (char*) arrayPtr, \
    .addr.array.arr.objects.stride = sizeof(arrayPtr[0]), \
    .addr.array.count = length, \
    .addr.array.maxLen = (int) (sizeof(arrayPtr) / sizeof(arrayPtr[0]))
