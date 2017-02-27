LEON_BASE = leon

#MPYTHON_INCLUDE += -I$(MPYTHON_BASE)
CC_INCLUDE += -I$(LEON_BASE)/build

mpython_headers:
	cd $(LEON_BASE) && $(MAKE)
	
# Set the path for the Motor Control API
MOTOR_CONTROL_BASE = MotorControl

# Motor control headers
include $(MOTOR_CONTROL_BASE)/motorcontrol.mk

#BUILD = leon/build
#MINIMAL_BASE = leon/build
#MPYTHON_INCLUDE += -I$(MINIMAL_BASE)

#CC_INCLUDE += $(MPYTHON_INCLUDE)
#MPYTHON_INCLUDE += -I$(BUILD)

#CFLAGS += $(MPYTHON_INCLUDE)


#CC_INCLUDE += 
# ------------------------------[ App ]--------------------------------------------#
# Default application name is the same as the folder it is in.
# This can be overridden here if something different is required
APPNAME ?= Micropython
# ------------------------------[ Build overrides ]--------------------------------#
# overrides that need to be set before including generic.mk
MV_SOC_PLATFORM = myriad2
MV_SOC_OS = rtems
RTEMS_BUILD_NAME = b-prebuilt
MV_SOC_REV = ma2100
CCOPT_EXTRA = -mlittle-endian 
# ------------------------------[ EoT Components used ]--------------------------------#
EoTComponentList = Crypto SDCardIO WifiFunctions TimeFunctions Camera libccv
# ------------------------------[ MDK Components used ]--------------------------------#
#ComponentList   := Board182 CameraModules CameraModules/imx208 CamGeneric
#MVSIMOPT        += -darw
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
