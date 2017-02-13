# ------------------------------[ App ]--------------------------------------------#
# Default application name is the same as the folder it is in.
# This can be overridden here if something different is required
APPNAME ?= Libccv_test
# ------------------------------[ Build overrides ]--------------------------------#
# overrides that need to be set before including generic.mk
MV_SOC_PLATFORM ?= myriad2
MV_SOC_REV      ?= ma2100

# Ensure that the we are using the correct rtems libs etc
MV_SOC_OS = rtems
RTEMS_BUILD_NAME = b-prebuilt
CCOPT_EXTRA = -mlittle-endian
# ------------------------------[ EoT Components used ]--------------------------------#
EoTComponentList = Crypto SDCardIO libccv #zlib cblas libpng 
#-------------------------------[ Settings ]---------------------------------------#
# Add a include path
#CC_INCLUDE += $(patsubst %,-I%,$(wildcard $(DirAppRoot)/../code/leon ) )
# Add source code that is not located in the default path
#LEON_APP_C_SOURCES += $(wildcard $(DirAppRoot)/../code/leon/*.c)
# Enable C99
#CCOPT += -std=gnu99
# Enable warnings
#CCOPT += -Wall -Wextra -Wpedantic
# Debug or not, uncomment for debugging
#DEBUG ?= yes
# Verbose or not, uncomment for more verbosity
#VERBOSE ?= yes
# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
MV_COMMON_BASE  ?= $(MV_TOOLS_DIR)/../mdk/common
# Set EOT_PACKAGES_BASE relative to mdk directory location (but allow user to override in environment)
EOT_PACKAGES_BASE ?= $(MV_TOOLS_DIR)/../mdk/packages/EoT
# Include the EoT Makefile
include $(EOT_PACKAGES_BASE)/EoT.mk
# Include the generic Makefile
include $(MV_COMMON_BASE)/generic.mk
