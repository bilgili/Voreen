# Adds "most" available modules.
# This means all modules that do not depend on external libraries that
# are not available by default or are currently broken.
#

# save original module list, so that "all cuda" works as expected
ORG=$$VRN_MODULES

# First include all modules
include(all.pri)
