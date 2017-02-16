# ------------------------------[ App ]--------------------------------------------#
APPNAME ?= TestFlash
# ------------------------------[ Build overrides ]--------------------------------#
# overrides that need to be set before including generic.mk
MV_SOC_PLATFORM = myriad2
# ------------------------------[ EoT Components used ]--------------------------------#
EoTComponentList = FlashIO embUnit
#-------------------------------[ Settings ]---------------------------------------#
# Verbose or not, uncomment for more verbosity
#VERBOSE ?= yes
# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
MV_COMMON_BASE  ?= $(MV_TOOLS_DIR)/../mdk/common
# Set EOT_PACKAGES_BASE relative to mdk directory location (but allow user to override in environment)
EOT_PACKAGES_BASE ?= $(MV_TOOLS_DIR)/../mdk/packages/EoT

# Include the generic Makefile
include $(MV_COMMON_BASE)/generic.mk
# Include the EoT Makefile
include $(EOT_PACKAGES_BASE)/EoT.mk

.PHONY: setup
setup:
	make flash_erase;
	make clean;
	make all;
	make flash;