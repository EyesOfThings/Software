# This makefile sets up the Motor Contorl API

#Get the path of the makefile
SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# Add the includes
CCOPT += -I$(SELF_DIR)

# Add the flag to indicate to use the myriad GPIO
CCOPT += -DMYRIAD

# Add the sources for the robots
LEON_APP_C_SOURCES +=$(shell find $(SELF_DIR) -name *.c)
