# Include the generic Makefile
#include $(MV_COMMON_BASE)/generic.mk
# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
#MV_COMMON_BASE  ?= $(MV_TOOLS_DIR)/../mdk/common
# Set EOT_PACKAGES_BASE relative to mdk directory location (but allow user to override in environment)
#EOT_PACKAGES_BASE ?= $(MV_TOOLS_DIR)/../mdk/packages/EoT
# Include the EoT Makefile
#include $(EOT_PACKAGES_BASE)/EoT.mk
# Include the generic Makefile
#include $(MV_COMMON_BASE)/generic.mk
# Define the locaiton of the sources
BUILD = build
PY_SRC = py
SRC_MOD = $(shell find eot -name *.c)

#MPYTHON_INCLUDE += -Ibuild
MPYTHON_INCLUDE += -I.
#MPYTHON_INCLUDE += -Ilib/mp-readline

#MPYTHON_INCLUDE += -Iminimal
#MPYTHON_INCLUDE += -Ilibc/mp_readline

# Add the include paths
CFLAGS += $(MPYTHON_INCLUDE)

# Include the micropython environment
include py/mkenv.mk
# Include the micropython makefile
# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

include py/py.mk

# Create a rule to preprocess the headers
all: $(BUILD)/genhdr/qstrdefs.generated.h
	$(info GEN $@)
	
# Include the rules from micropython
include py/mkrules.mk
