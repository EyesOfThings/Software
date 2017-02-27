/*
 * This implementation is based on MircoJSON 1.3, which can be found unmodified
 * at: http://www.catb.org/esr/microjson/
 * This is the original copyright notice:
 * This file is Copyright (c) 2014 by Eric S. Raymond
 * BSD terms apply: see the file COPYING for details.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mjson.h"

enum ErrorCode {
    JSON_ERR_OBSTART     =  1,  // non-WS when expecting object start
    JSON_ERR_ATTRSTART   =  2,  // non-WS when expecting attribute start
    JSON_ERR_BADATTR     =  3,  // unknown attribute name
    JSON_ERR_NOARRAY     =  4,  // saw [ when not expecting array
    JSON_ERR_NOBRAK      =  5,  // array element specified, but no [
    JSON_ERR_STRLONG     =  6,  // string value too long
    JSON_ERR_BADTRAIL    =  7,  // garbage while expecting comma or } or ]
    JSON_ERR_ARRAYSTART  =  8,  // didn't find expected array start
    JSON_ERR_SUBTOOLONG  =  9,  // too many array elements
    JSON_ERR_BADSUBTRAIL = 10,  // garbage while expecting array comma
    JSON_ERR_SUBTYPE     = 11,  // unsupported array element type
    JSON_ERR_BADSTRING   = 12,  // error while string parsing
    JSON_ERR_NOPARSTR    = 13,  // can't support strings in parallel arrays
    JSON_ERR_BADENUM     = 14,  // invalid enumerated value
    JSON_ERR_QNONSTRING  = 15,  // saw quoted value when expecting non-string
    JSON_ERR_NONQSTRING  = 16,  // didn't see quoted value when expecting string
    JSON_ERR_BADNUM      = 17,  // error while parsing a numerical argument
    JSON_ERR_NULLPTR     = 18,  // unexpected null value or attribute pointer
};

static char* json_target_address(const struct json_attr_t* cursor,
                                 const struct json_array_t* parent,
                                 int offset)
{
    char* targetAddress = NULL;
    if (parent == NULL || parent->element_type != t_structobject)
    {
        // ordinary case - use the address in the cursor structure
        switch (cursor->type)
        {
        case t_ignore:
            targetAddress = NULL;
            break;
        case t_integer:
            targetAddress = (char*) &cursor->addr.integer[offset];
            break;
        case t_uinteger:
            targetAddress = (char*) &cursor->addr.uinteger[offset];
            break;
        case t_float:
            targetAddress = (char*) &cursor->addr.real[offset];
            break;
        case t_string:
            targetAddress = cursor->addr.string;
            break;
        case t_boolean:
            targetAddress = (char*) &cursor->addr.boolean[offset];
            break;
        case t_character:
            targetAddress = &cursor->addr.character[offset];
            break;
        default:
            targetAddress = NULL;
            break;
        }
    }
    else
    {
        // tricky case - hacking a member in an array of structures
        targetAddress = parent->arr.objects.base +
                        (offset * parent->arr.objects.stride) +
                        cursor->addr.offset;
    }
    return targetAddress;
}

static int json_internal_read_object(const char* cp,
                                     const struct json_attr_t* attributes,
                                     const struct json_array_t* parent,
                                     int offset,
                                     const char** end)
{
    enum
    {
        init = 0,
        await_attr,
        in_attr,
        await_value,
        in_val_string,
        in_val_token,
        post_val,
        post_array
    } state = 0;

    const char* attrPtr = NULL;
    const char* valPtr = NULL;
    char tokenBuf[12];
    bool valueQuoted = false;
    const struct json_attr_t* cursor;
    const struct json_enum_t* mapPtr;
    size_t attrLen = 0;
    size_t valueLen = 0;

    if (end != NULL)
        *end = NULL;  // give it a well-defined value on parse failure

    // fill fields with defaults in case they are omitted in the JSON input
    for (cursor = attributes; cursor->attribute != NULL; ++cursor)
    {
        if (!cursor->noDefault)
        {
            char* lPtr = json_target_address(cursor, parent, offset);
            if (lPtr != NULL)
            {
                switch (cursor->type)
                {
                case t_integer:
                    memcpy(lPtr, &cursor->defaultValue.integer, sizeof(int));
                    break;
                case t_uinteger:
                    memcpy(lPtr, &cursor->defaultValue.uinteger,
                           sizeof(unsigned int));
                    break;
                case t_float:
                    memcpy(lPtr, &cursor->defaultValue.real, sizeof(float));
                    break;
                case t_string:
                    if (parent != NULL &&
                        parent->element_type != t_structobject && offset > 0)
                    {
                        return JSON_ERR_NOPARSTR;
                    }
                    lPtr[0] = '\0';
                    break;
                case t_boolean:
                    memcpy(lPtr, &cursor->defaultValue.boolean, sizeof(bool));
                    break;
                case t_character:
                    lPtr[0] = cursor->defaultValue.character;
                    break;
                case t_object:    // silences a compiler warning
                case t_structobject:
                case t_array:
                case t_ignore:
                    break;
                }
            }
        }
    }

    // parse input JSON
    for (; *cp != '\0'; ++cp)
    {
        switch (state)
        {
        case init:
            if (isspace((unsigned char) *cp))
                continue;
            else if (*cp == '{')
                state = await_attr;
            else
            {
                if (end != NULL)
                    *end = cp;
                return JSON_ERR_OBSTART;
            }
            break;
        case await_attr:
            if (isspace((unsigned char) *cp))
                continue;
            else if (*cp == '"')
            {
                state = in_attr;
                attrLen = 0;
                attrPtr = cp + 1;
                if (end != NULL)
                    *end = cp;
            }
            else if (*cp == '}')
                break;
            else
            {
                if (end != NULL)
                    *end = cp;
                return JSON_ERR_ATTRSTART;
            }
            break;
        case in_attr:
            if (attrPtr == NULL)
                // don't update end here, leave at attribute start
                return JSON_ERR_NULLPTR;
            if (*cp == '"')
            {
                for (cursor = attributes; cursor->attribute != NULL; ++cursor)
                {
                    size_t refAttrLen = strlen(cursor->attribute);
                    if (refAttrLen == attrLen &&
                        strncmp(cursor->attribute, attrPtr, refAttrLen) == 0)
                    {
                        break;
                    }
                }
                if (cursor->attribute == NULL)
                {
                    // don't update end here, leave at attribute start
                    return JSON_ERR_BADATTR;
                }
                state = await_value;
            }
            else
            {
                ++attrLen;
            }
            break;
        case await_value:
            if (isspace((unsigned char) *cp) || *cp == ':')
                continue;
            else if (*cp == '[')
            {
                if (cursor->type != t_array)
                {
                    if (end != NULL)
                        *end = cp;
                    return JSON_ERR_NOARRAY;
                }
                int subStatus = json_read_array(cp, &cursor->addr.array, &cp);
                if (subStatus != 0)
                    return subStatus;
                state = post_array;
            }
            else if (cursor->type == t_array)
            {
                if (end != NULL)
                    *end = cp;
                return JSON_ERR_NOBRAK;
            }
            else if (*cp == '"')
            {
                valueQuoted = true;
                state = in_val_string;
                valueLen = 0;
                valPtr = cp + 1;
            }
            else
            {
                valueQuoted = false;
                state = in_val_token;
                valueLen = 1;
                valPtr = cp;
            }
            break;
        case in_val_string:
            if (valPtr == NULL)
                // don't update end here, leave at value start
                return JSON_ERR_NULLPTR;
            if (*cp == '"')
            {
                state = post_val;
            }
            else if (cursor->type == t_string && valueLen >= cursor->len)
            {
                // don't update end here, leave at value start
                return JSON_ERR_STRLONG;
            }
            else
                ++valueLen;
            break;
        case in_val_token:
            if (valPtr == NULL)
                // don't update end here, leave at value start
                return JSON_ERR_NULLPTR;
            if (isspace((unsigned char) *cp) || *cp == ',' || *cp == '}')
            {
                state = post_val;
                if (*cp == '}' || *cp == ',')
                    --cp;
            }
            else
                ++valueLen;
            break;
        case post_val:
            /*
            * We know that cursor points at the first spec matching
            * the current attribute.  We don't know that it's *the*
            * correct spec; our dialect allows there to be any number
            * of adjacent ones with the same attrname but different
            * types.  Here's where we try to seek forward for a
            * matching type/attr pair if we're not looking at one.
            */
            while (true)
            {
                int seeking = cursor->type;
                if (valueQuoted && (cursor->type == t_string))
                    break;
                if (((valueLen == 4 && strncmp(valPtr, "true", valueLen) == 0) ||
                     (valueLen == 5 && strncmp(valPtr, "false", valueLen) == 0)) &&
                       seeking == t_boolean)
                {
                    break;
                }
                if (isdigit((unsigned char) *valPtr))
                {
                    bool decimal = false;
                    for (unsigned int i = 0; i < valueLen; ++i)
                    {
                        if (*(valPtr + i) == '.')
                        {
                            decimal = true;
                            break;
                        }
                    }
                    if (decimal && seeking == t_float)
                        break;
                    if (!decimal &&
                        (seeking == t_integer || seeking == t_uinteger))
                    {
                        break;
                    }
                }
                if (cursor[1].attribute == NULL)  // out of possibilities
                    break;
                if (strlen(cursor[1].attribute) != attrLen ||
                    strncmp(cursor[1].attribute, attrPtr, attrLen) != 0)
                {
                    break;
                }
                ++cursor;
            }
            if (valueQuoted
                && (cursor->type != t_string && cursor->type != t_character
                    && cursor->type != t_ignore && cursor->map == 0))
            {
                return JSON_ERR_QNONSTRING;
            }
            if (!valueQuoted && (cursor->type == t_string || cursor->map != 0))
            {
                return JSON_ERR_NONQSTRING;
            }
            if (cursor->map != NULL)
            {
                for (mapPtr = cursor->map; mapPtr->name != NULL; ++mapPtr)
                {
                    if (strncmp(mapPtr->name, valPtr, valueLen) == 0)
                    {
                        goto foundit;
                    }
                }
                return JSON_ERR_BADENUM;
                foundit:
                snprintf(tokenBuf, sizeof(tokenBuf), "%d", mapPtr->value);
                tokenBuf[sizeof(tokenBuf) - 1] = '\0';
                valPtr = &tokenBuf[0];
            }
            char* lPtr = json_target_address(cursor, parent, offset);
            if (lPtr != NULL)
            {
                switch (cursor->type)
                {
                case t_integer:
                {
                    int tmp = atoi(valPtr);
                    memcpy(lPtr, &tmp, sizeof(int));
                    break;
                }
                case t_uinteger:
                {
                    unsigned int tmp = (unsigned int) atoi(valPtr);
                    memcpy(lPtr, &tmp, sizeof(unsigned int));
                    break;
                }
                case t_float:
                {
                    float tmp = strtof(valPtr, NULL);
                    memcpy(lPtr, &tmp, sizeof(float));
                    break;
                }
                case t_string:
                {
                    if (parent != NULL &&
                        parent->element_type != t_structobject && offset > 0)
                    {
                        return JSON_ERR_NOPARSTR;
                    }
                    strncpy(lPtr, valPtr,
                            (cursor->len < valueLen) ? cursor->len : valueLen);
                    lPtr[(cursor->len < valueLen) ? cursor->len : valueLen] = '\0';
                    break;
                }
                case t_boolean:
                {
                    bool tmp = (strncmp(valPtr, "true", valueLen) == 0);
                    memcpy(lPtr, &tmp, sizeof(bool));
                    break;
                }
                case t_character:
                    if (valueLen > 1)
                        // don't update end here, leave at value start
                        return JSON_ERR_STRLONG;
                    else
                        *lPtr = *valPtr;
                    break;
                case t_ignore:    // silences a compiler warning
                case t_object:    // silences a compiler warning
                case t_structobject:
                case t_array:
                    break;
                }
            }
            // fall through
        case post_array:
            if (isspace((unsigned char) *cp))
                continue;
            else if (*cp == ',')
                state = await_attr;
            else if (*cp == '}')
            {
                ++cp;
                goto good_parse;
            }
            else
            {
                if (end != NULL)
                    *end = cp;
                return JSON_ERR_BADTRAIL;
            }
            break;
        }
    }

    good_parse:
    // in case there is another object following, consume trailing whitespace
    while (isspace((unsigned char) *cp))
        ++cp;
    if (end != NULL)
        *end = cp;
    return 0;
}

int json_read_array(const char* cp,
                    const struct json_array_t* arr,
                    const char** end)
{
    if (end != NULL)
        *end = NULL;  // give it a well-defined value on parse failure

    while (isspace((unsigned char) *cp))
        ++cp;
    if (*cp != '[')
        return JSON_ERR_ARRAYSTART;
    else
        ++cp;

    char* tp = arr->arr.strings.store;
    int arrCount = 0;

    // Check for empty array
    while (isspace((unsigned char) *cp))
        ++cp;
    if (*cp == ']')
        goto breakout;

    for (int offset = 0; offset < arr->maxLen; ++offset)
    {
        char* ep = NULL;
        switch (arr->element_type)
        {
        case t_string:
            if (isspace((unsigned char) *cp))
                ++cp;
            if (*cp != '"')
                return JSON_ERR_BADSTRING;
            else
                ++cp;
            arr->arr.strings.ptrs[offset] = tp;
            for (; tp - arr->arr.strings.store < arr->arr.strings.storeLen; ++tp)
            {
                if (*cp == '"')
                {
                    ++cp;
                    *tp++ = '\0';
                    goto stringEnd;
                }
                else if (*cp == '\0')
                {
                    return JSON_ERR_BADSTRING;
                }
                else
                {
                    *tp = *cp++;
                }
            }
            return JSON_ERR_BADSTRING;
        stringEnd:
            break;
        case t_object:
        case t_structobject:
        {
            int subStatus = json_internal_read_object(cp,
                    arr->arr.objects.subtype, arr, offset, &cp);
            if (subStatus != 0)
            {
                if (end != NULL)
                    end = &cp;
                return subStatus;
            }
            break;
        }
        case t_integer:
            arr->arr.integers.store[offset] = (int) strtol(cp, &ep, 0);
            if (ep == cp)
                return JSON_ERR_BADNUM;
            else
                cp = ep;
            break;
        case t_uinteger:
            arr->arr.uintegers.store[offset] = (unsigned int) strtoul(cp, &ep, 0);
            if (ep == cp)
                return JSON_ERR_BADNUM;
            else
                cp = ep;
            break;
        case t_float:
            arr->arr.reals.store[offset] = strtof(cp, &ep);
            if (ep == cp)
                return JSON_ERR_BADNUM;
            else
                cp = ep;
            break;
        case t_boolean:
            if (strncmp(cp, "true", 4) == 0)
            {
                arr->arr.booleans.store[offset] = true;
                cp += 4;
            }
            else if (strncmp(cp, "false", 5) == 0)
            {
                arr->arr.booleans.store[offset] = false;
                cp += 5;
            }
            break;
        case t_character:
        case t_array:
        case t_ignore:
            return JSON_ERR_SUBTYPE;
        }
        ++arrCount;
        if (isspace((unsigned char) *cp))
           ++cp;
        if (*cp == ']')
        {
            goto breakout;
        }
        else if (*cp == ',')
        {
            cp++;
        }
        else
        {
            return JSON_ERR_BADSUBTRAIL;
        }
    }
    if (end != NULL)
        *end = cp;
    return JSON_ERR_SUBTOOLONG;

    breakout:
    if (arr->count != NULL)
        *(arr->count) = arrCount;
    if (end != NULL)
        *end = cp;
    return 0;
}

int json_read_object(const char* cp,
                     const struct json_attr_t* attributes,
                     const char** end)
{
    return json_internal_read_object(cp, attributes, NULL, 0, end);
}

const char* json_error_string(int err)
{
    const char* errors[] =
    {
		"unknown error while parsing JSON",
		"non-whitespace when expecting object start",
		"non-whitespace when expecting attribute start",
		"unknown attribute name",
		"saw [ when not expecting array",
		"array element specified, but no [",
		"string value too long",
		"garbage while expecting comma or } or ]",
		"didn't find expected array start",
		"too many array elements",
		"garbage while expecting array comma",
		"unsupported array element type",
		"error during string parsing",
		"strings in parallel arrays are not supported",
		"invalid enumerated value",
		"saw quoted value when expecting non-string",
		"didn't see quoted value when expecting string",
		"error while parsing a numerical argument",
		"unexpected null value or attribute pointer"
    };

    if (err <= 0 || err >= (int) (sizeof(errors) / sizeof(errors[0])))
        return errors[0];
    else
        return errors[err];
}
