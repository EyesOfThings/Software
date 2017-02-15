# ------------------------------[ App ]--------------------------------------------#
# Default application name is the same as the folder it is in.
# This can be overridden here if something different is required
#APPNAME ?= $(notdir $(shell pwd))

MV_SOC_PLATFORM ?= myriad2
MV_SOC_REV      ?= ma2100

# Ensure that the we are using the correct rtems libs etc
MV_SOC_OS = rtems
RTEMS_BUILD_NAME = b-prebuilt
CCOPT_EXTRA = -mlittle-endian

EoTComponentList = HistogramMatching

# ------------------------------[ Tools ]------------------------------------------#
# Hardcode tool version here if needed, otherwise defaults to Latest
#MV_TOOLS_VERSION ?=

# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
MV_COMMON_BASE  ?= $(MV_TOOLS_DIR)/../mdk/common

# Set EOT_PACKAGES_BASE relative to mdk directory location
# (but allow user to override in environment)
EOT_PACKAGES_BASE ?= $(MV_TOOLS_DIR)/../mdk/packages/EoT
# Include the EoT Makefile
include $(EOT_PACKAGES_BASE)/EoT.mk
# Include the generic Makefile
include $(MV_COMMON_BASE)/generic.mk
# -------------------------------- [ Build Options ] ------------------------------ #
# App related build options

# Extra app related options
#CCOPT += -DNAME=VALUE
#CCOPT += -MD
#Don't clear BSS. This speeds up RTEMS a log
#CCOPT += -DDONT_CLEAR_BSS
#CCOPT += -DDISABLE_LEON_ICACHE -DDISABLE_LEON_DCACHE

TEST_TYPE        := AUTO
TEST_TAGS        := "MA2100, MA2150"
