# ------------------------------[ App ]--------------------------------------------#
APPNAME ?= TestSDCardIO
# ------------------------------[ Build overrides ]--------------------------------#
# overrides that need to be set before including generic.mk
MV_SOC_PLATFORM = myriad2
MV_SOC_OS = rtems
# ------------------------------[ EoT Components used ]--------------------------------#
EoTComponentList = SDCardIO embUnit
#-------------------------------[ Settings ]---------------------------------------#
# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
MV_COMMON_BASE  ?= $(MV_TOOLS_DIR)/../mdk/common
# Set EOT_PACKAGES_BASE relative to mdk directory location (but allow user to override in environment)
EOT_PACKAGES_BASE ?= $(MV_TOOLS_DIR)/../mdk/packages/EoT

# Include the generic Makefile
include $(MV_COMMON_BASE)/generic.mk
# Include the EoT Makefile
include $(EOT_PACKAGES_BASE)/EoT.mk