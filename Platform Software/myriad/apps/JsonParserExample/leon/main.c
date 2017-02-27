#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtems.h>
#include <mv_types.h>
#include <mjson.h>
#include "rtems_config.h"

static pthread_attr_t mainThreadAttr;
static pthread_t mainThread;

struct example_struct_t
{
    int exampleInteger;
    unsigned int exampleUnsignedInteger;
    float exampleFloat;
    char exampleString[1024];
    bool exampleBoolean;
    char exampleChar;
    int exampleToken;
};

struct example_array_t
{
    int size;
    struct example_struct_t list[3];
};

static struct example_array_t exampleList;

static int parseExampleJson(const char* buf, struct json_enum_t* map)
{
    const struct json_attr_t jsonElementAttrs[] =
    {
        {"exampleIntAttr", t_integer,
                STRUCTOBJECT(struct example_struct_t, exampleInteger)},
        {"exampleUIntAttr", t_uinteger,
                STRUCTOBJECT(struct example_struct_t, exampleUnsignedInteger)},
        {"exampleFloatAttr", t_float,
                STRUCTOBJECT(struct example_struct_t, exampleFloat)},
        {"exampleStringAttr", t_string,
                STRUCTOBJECT(struct example_struct_t, exampleString),
                .len = sizeof(exampleList.list[0].exampleString)},
        {"exampleBoolAttr", t_boolean,
                STRUCTOBJECT(struct example_struct_t, exampleBoolean)},
        {"exampleCharAttr", t_character,
                STRUCTOBJECT(struct example_struct_t, exampleChar)},
		{"exampleTokenAttr", t_integer,
				STRUCTOBJECT(struct example_struct_t, exampleToken), .map = map},
        {NULL}
    };
    const struct json_attr_t jsonListAttrs[] =
    {
        {"exampleListAttr", t_array, STRUCTARRAY(exampleList.list, jsonElementAttrs, &exampleList.size)},
        {NULL}
    };
    memset(&exampleList, '\0', sizeof(exampleList));
    int status = json_read_object(buf, jsonListAttrs, NULL);
    return status;
}

void* parserTest(void* args)
{
    UNUSED(args);
    char test[] = "{\"exampleListAttr\":["
					   "{"
    						"\"exampleIntAttr\":-7,"
                            "\"exampleUIntAttr\":1411468340,"
                            "\"exampleFloatAttr\":4.3f,"
                            "\"exampleStringAttr\":\"test string\","
                            "\"exampleCharAttr\":\"c\","
                            "\"exampleBoolAttr\":true,"
    						"\"exampleTokenAttr\":\"exampleTokenOne\""
    					"},"
                        "{"
                        	"\"exampleIntAttr\":734,"
                            "\"exampleUIntAttr\":269341,"
                            "\"exampleFloatAttr\":-314.2f,"
                            "\"exampleStringAttr\":\"foo bar\","
                            "\"exampleCharAttr\":\"z\","
                            "\"exampleBoolAttr\":false,"
    						"\"exampleTokenAttr\":\"exampleTokenTwo\""
						"}"
                  "]}";
    struct json_enum_t tokenMap[2];
    tokenMap[0].name = "exampleTokenOne";
    tokenMap[0].value = 1;
    tokenMap[1].name = "exampleTokenTwo";
    tokenMap[1].value = 2;
    int status = parseExampleJson(test, tokenMap);

    if (status != 0)
    {
        puts(json_error_string(status));
    }
    else
    {
		printf("no. of elements in array: %d\n", exampleList.size);

		for (int i = 0; i < exampleList.size; i++)
		{
			printf("attributes of element %i:\n", i);
			printf("exampleIntAttr : %i\n", exampleList.list[i].exampleInteger);
			printf("exampleUIntAttr : %u\n", exampleList.list[i].exampleUnsignedInteger);
			printf("exampleFloatAttr : %f\n", exampleList.list[i].exampleFloat);
			printf("exampleStringAttr : %s\n", exampleList.list[i].exampleString);
			printf("exampleCharAttr : %c\n", exampleList.list[i].exampleChar);
			printf("exampleBoolAttr : %i\n", exampleList.list[i].exampleBoolean);
			printf("exampleTokenAttr : %i", exampleList.list[i].exampleToken);
			printf("\n");
		}
    }
    pthread_exit(0);
    return NULL;
}

void POSIX_Init(void* args)
{
    UNUSED(args);

    int sc = initClocksAndMemory();
    if (sc)
        exit(sc);

    printk("\nRTEMS POSIX Started\n");

    if (pthread_attr_init(&mainThreadAttr))
    {
        printk("failed to init thread attributes\n");
        exit(1);
    }
    if (pthread_attr_setinheritsched(&mainThreadAttr, PTHREAD_EXPLICIT_SCHED))
    {
        printk("failed to set thread schedule\n");
        exit(1);
    }
    if (pthread_attr_setschedpolicy(&mainThreadAttr, SCHED_RR))
    {
        printk("failed to set thread schedule policy\n");
        exit(1);
    }
    if (pthread_create(&mainThread, &mainThreadAttr, parserTest, NULL))
    {
    	printk("failed to create thread\n");
		exit(1);
    }
    if (pthread_join(mainThread, NULL))
    {
    	printk("failed to join thread\n");
		exit(1);
    }
    exit(0);
}

// User extension to be able to catch abnormal terminations
static void Fatal_extension(Internal_errors_Source the_source,
                            bool is_internal,
                            uint32_t the_error)
{
    switch (the_source)
    {
    case RTEMS_FATAL_SOURCE_EXIT:
        if (the_error)
            printk("Exited with error code %d\n", the_error);
        break; // normal exit
    case RTEMS_FATAL_SOURCE_ASSERT:
        printk("%s : %d in %s \n",
               ((rtems_assert_context*) the_error)->file,
               ((rtems_assert_context*) the_error)->line,
               ((rtems_assert_context*) the_error)->function);
        break;
    case RTEMS_FATAL_SOURCE_EXCEPTION:
        rtems_exception_frame_print((const rtems_exception_frame*) the_error);
        break;
    default:
        printk("\nSource %d Internal %d Error %d  0x%X:\n",
               the_source,
               is_internal, the_error, the_error);
        break;
    }
}
