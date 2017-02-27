# ------------------------------[ Build overrides ]--------------------------------#
# overrides that need to be set before including generic.mk
MV_SOC_PLATFORM ?= myriad2
# ------------------------------[ Components used ]--------------------------------#
#Component lists: 
#No components used by HelloWorldLeon
#Choosing if this project has shave components or not
SHAVE_COMPONENTS = no

#-------------------------------[ Settings ]---------------------------------------#

#-------------------------------[ Local shave applications sources ]--------------------#
#No shave code

#--------------------------[ Shave applications section ]--------------------------#
#NO SHAVE CODE

# ------------------------------[ Tools overrides ]--------------------------------#
# Hardcode tool version here if needed, otherwise defaults to latest stable release
#MV_TOOLS_VERSION ?=

# Verbose or not, uncomment for more verbosity
#ECHO ?=

# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
MV_COMMON_BASE  ?= ../../../common

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
TEST_TAGS := "MA2100, MA2150"
