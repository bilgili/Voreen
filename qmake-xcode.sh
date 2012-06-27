#!/bin/bash
#
# (Re)-Generates Xcode project files for voreen-core, voreen-qt, voreenve and voltool.
#
# Execute within Voreen home directory.
#
cd src/core
rm -rf voreen_core.xcodeproj
qmake -spec macx-xcode voreenlib_core.pro

####### Hack to disable "Always search user paths" in XCode
cd voreen_core.xcodeproj
mv project.pbxproj project.pbxproj.tmp
sed -e 's/HEADER_SEARCH_PATHS/ALWAYS_SEARCH_USER_PATHS = NO;\
				HEADER_SEARCH_PATHS/' project.pbxproj.tmp > project.pbxproj
rm project.pbxproj.tmp
cd ..
####### end of hack

cd ../qt
rm -rf voreen_qt.xcodeproj
qmake -spec macx-xcode voreenlib_qt.pro
cd ../../apps/voreenve
rm -rf voreenve.xcodeproj
qmake -spec macx-xcode voreenve.pro
cd ../../apps/voltool
rm -rf voltool.xcodeproj
qmake -spec macx-xcode voltool.pro

