# ------------------------------[ App ]--------------------------------------------#
# Default application name is the same as the folder it is in.
# This can be overridden here if something different is required
#APPNAME ?= RIFA

# ------------------------------[ Build overrides ]--------------------------------#
# overrides that need to be set before including generic.mk
MV_SOC_PLATFORM ?= myriad2
MV_SOC_REV      ?= ma2100

# Ensure that the we are using the correct rtems libs etc
MV_SOC_OS = rtems
RTEMS_BUILD_NAME = b-prebuilt
CCOPT_EXTRA = -mlittle-endian

# ------------------------------[ Components used ]--------------------------------#
ComponentList = Board182

EoTComponentList = Camera RTSP SDCardIO libccv OpenCV

#Choosing if this project has shave components or not
SHAVE_COMPONENTS = no

#-------------------------------[ Settings ]---------------------------------------#

#-------------------------------[ Local shave applications sources ]--------------------#
bbfParallelApp = shave/bbfParallel
SHAVE_C_SOURCES_bbfParallel = $(wildcard $(DirAppRoot)/shave/*.c)

#Generating list of required generated assembly files for the faceDetectionSHAVEs app on shave
SHAVE_GENASMS_bbfParallel = $(patsubst %.c,$(DirAppObjBase)%.asmgen,$(SHAVE_C_SOURCES_bbfParallel))
#Generating required objects list from sources
SHAVE_bbfParallel_OBJS = $(patsubst $(DirAppObjBase)%.asmgen,$(DirAppObjBase)%_shave.o,$(SHAVE_GENASMS_bbfParallel))

#update clean rules with our generated files
PROJECTCLEAN += $(SHAVE_GENASMS_bbfParallel) $(SHAVE_bbfParallel_OBJS) $(SHAVE0_APPS) $(SHAVE_APP_LIBS) $(bbfParallelApp).mvlib $(bbfParallelApp).shv0lib $(SHAVE1_APPS) $(SHAVE2_APPS) $(SHAVE3_APPS) $(SHAVE4_APPS) $(SHAVE5_APPS) $(SHAVE6_APPS) $(SHAVE7_APPS) $(SHAVE8_APPS) 
#Uncomment below to reject generated shave as intermediary files (consider them secondary)
PROJECTINTERM += $(SHAVE_GENASMS_bbfParallel)

SHAVE_APP_LIBS = $(bbfParallelApp).mvlib
SHAVE0_APPS = $(bbfParallelApp).shv0lib
SHAVE1_APPS = $(bbfParallelApp).shv1lib
SHAVE2_APPS = $(bbfParallelApp).shv2lib
SHAVE3_APPS = $(bbfParallelApp).shv3lib
SHAVE4_APPS = $(bbfParallelApp).shv4lib
SHAVE5_APPS = $(bbfParallelApp).shv5lib
SHAVE6_APPS = $(bbfParallelApp).shv6lib
SHAVE7_APPS = $(bbfParallelApp).shv7lib
SHAVE8_APPS = $(bbfParallelApp).shv8lib
#--------------------------[ Shave applications section ]--------------------------#
#NO SHAVE CODE

# ------------------------------[ Tools overrides ]--------------------------------#
# Hardcode tool version here if needed, otherwise defaults to latest stable release
#MV_TOOLS_VERSION =Latest

# Verbose or not, uncomment for more verbosity
#ECHO=

# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
MV_COMMON_BASE  ?= $(MV_TOOLS_DIR)/../mdk/common
# Set EOT_PACKAGES_BASE relative to mdk directory location (but allow user to override in environment)
EOT_PACKAGES_BASE ?= $(MV_TOOLS_DIR)/../mdk/packages/EoT
# Include the EoT Makefile
include $(EOT_PACKAGES_BASE)/EoT.mk
# Include the generic Makefile
include $(MV_COMMON_BASE)/generic.mk

#-------------------------------[ Local shave applications build rules ]------------------#
#Describe the rule for building theApp application. Simple rule specifying 
#which objects build up the said application. The application will be built into a library

# -------------------------------- [ Build Options ] ------------------------------ #
# App related build options 

# Extra app related options
#CCOPT			+= -DDEBUG
CCOPT           += 
#TEST_TAGS := "MA2100, MA2150, MDK_COMPONENTS"

ENTRYPOINTS = -e bbfParallel  --gc-sections

$(bbfParallelApp).mvlib : $(SHAVE_bbfParallel_OBJS) $(PROJECT_SHAVE_LIBS)
	$(ECHO) $(LD) -r $(ENTRYPOINTS) $(MVLIBOPT) $(SHAVE_bbfParallel_OBJS) $(PROJECT_SHAVE_LIBS) $(CompilerANSILibs) -o $@
