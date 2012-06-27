#!/bin/bash
#
# (Re)-Generates Xcode project files for voreen-core, voreen-qt and
# voreenve.
#
# Execute within Voreen home directory.
#
cd src/core
rm -rf voreen_core.xcodeproj
qmake -spec macx-xcode voreenlib_core.pro
cd ../qt
rm -rf voreen_qt.xcodeproj
qmake -spec macx-xcode voreenlib_qt.pro
cd ../../apps/voreenve
rm -rf voreenve.xcodeproj
qmake -spec macx-xcode voreenve.pro
