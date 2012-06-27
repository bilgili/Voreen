#!/bin/bash

######################################################################
#                                                                    #
# Voreen - Mac deployment script                                     #
#                                                                    #
# Copyright (C) 2005-2009 Visualization and Computer Graphics Group, #
# Department of Computer Science, University of Muenster, Germany.   #
# <http://viscg.uni-muenster.de>                                     #
#                                                                    #
######################################################################

# Configuration
DEPLOYMENT_TOOL="../../tools/macdeployment/macdeployqt-voreen/macdeployqt-voreen"

APP_BUNDLE_NAME="voreenve.app"
APP_BUNDLE_PATH="."
APP_BINARY_NAME="voreenve"
APP_BUNDLE_DEPLOYMENT_PATH="VoreenVE"

VOLTOOL_APP_BUNDLE_NAME="voltool.app"
VOLTOOL_APP_BUNDLE_PATH="../voltool"
VOLTOOL_BINARY_NAME="voltool"

DMG_NAME="VoreenVE"

SHADERS_PATH="../../src/core/vis/glsl"



# Delete previous dmg and dist directory
rm $DMG_NAME.dmg 2> /dev/null
rm -rf dist 2> /dev/null



# Deploy app bundle
echo ""
echo "* ---------------------------------------------------"
echo "*  Deploying dependencies of $APP_BUNDLE_NAME ...    "
echo "* ---------------------------------------------------"

# Create temporary distribution directory 'dist' and copy app bundle to it
echo ""
echo "* Creating temporary distribution directory and copying over app bundle ..."
mkdir -v dist
mkdir -v dist/$APP_BUNDLE_DEPLOYMENT_PATH
cp -Rv $APP_BUNDLE_PATH/$APP_BUNDLE_NAME dist/$APP_BUNDLE_DEPLOYMENT_PATH

# Call deployment tool on app bundle (copies dependencies into the app bundle and registers them)
$DEPLOYMENT_TOOL dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME

# deployment tool does currently only consider Qt frameworks and dylibs, but no other frameworks
# => custom frameworks have to be deployed separately
echo ""
echo "* Deploying additional frameworks to dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME ..."
# Python
cp -v /System/Library/Frameworks/Python.framework/Versions/2.5/Python dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Frameworks
install_name_tool -id @executable_path/../Frameworks/python dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Frameworks/python
install_name_tool -change /System/Library/Frameworks/Python.framework/Versions/2.5/Python @executable_path/../Frameworks/python dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/MacOS/$APP_BINARY_NAME



# Deploy voltool (analogous to deployment of app bundle)
echo ""
echo "* --------------------------------------------------------"
echo "*  Deploying dependencies of $VOLTOOL_APP_BUNDLE_NAME ... "
echo "* --------------------------------------------------------"
echo ""

echo "Copying over $VOLTOOL_APP_BUNDLE_NAME to temporary directory..."
cp -Rv $VOLTOOL_APP_BUNDLE_PATH/$VOLTOOL_APP_BUNDLE_NAME dist

# deployment tool
$DEPLOYMENT_TOOL dist/$VOLTOOL_APP_BUNDLE_NAME

# frameworks (Python)
echo ""
echo "* Deploying additional frameworks to dist/$VOLTOOL_APP_BUNDLE_NAME ..."
cp -v /System/Library/Frameworks/Python.framework/Versions/2.5/Python dist/$VOLTOOL_APP_BUNDLE_NAME/Contents/Frameworks
install_name_tool -id @executable_path/../Frameworks/python dist/$VOLTOOL_APP_BUNDLE_NAME/Contents/Frameworks/python
install_name_tool -change /System/Library/Frameworks/Python.framework/Versions/2.5/Python @executable_path/../Frameworks/python dist/$VOLTOOL_APP_BUNDLE_NAME/Contents/MacOS/$VOLTOOL_BINARY_NAME

# copy over voltool binary to main app bundle
echo "* Copying over voltool binary to VoreenVE app bundle ..."
mkdir -v dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Tools
cp -v dist/$VOLTOOL_APP_BUNDLE_NAME/Contents/MacOS/voltool dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Tools
# remove voltool app bundle
echo "* Deleting voltool app bundle dist/$VOLTOOL_APP_BUNDLE_NAME ..."
rm -rf dist/$VOLTOOL_APP_BUNDLE_NAME



echo ""
echo "* --------------------------------------------------------"
echo "*  Copying over voreen resources to $APP_BUNDLE_NAME ...  "
echo "* --------------------------------------------------------"
#
# Copy readonly resources to app bundle's resources path:
# - shaders.tar
# - textures
# - fonts
# - documentation 
#
echo "* Generating shaders.tar (dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources/shaders.tar) ..."
mkdir dist/shaders
rsync -r --exclude=.svn $SHADERS_PATH/ dist/shaders/
cd dist/shaders
tar -cvf ../$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources/shaders.tar *
cd ..
rm -rf shaders
cd ..
echo ""

mkdir -v dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources/textures
rsync -rv --exclude=.svn ../../data/textures/ dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources/textures/
mkdir -v dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources/fonts/
rsync -rv --exclude=.svn ../../data/fonts/ dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources/fonts/
mkdir -v dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources/doc/
rsync -rv --exclude=.svn ../../doc/ dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources/doc/

# copy over Info.plist and icon
cp -v ../../tools/macdeployment/Info.plist dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents
cp -v ../../tools/macdeployment/icon.icns dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Resources

#
# add user data to subdir 'data' within deployment path 
#
mkdir -v dist/$APP_BUNDLE_DEPLOYMENT_PATH/data

# copy volumes to data dir
cp -v ../../data/nucleon.dat dist/$APP_BUNDLE_DEPLOYMENT_PATH/data
cp -v ../../data/nucleon.raw dist/$APP_BUNDLE_DEPLOYMENT_PATH/data
cp -v ../../data/walnut.dat dist/$APP_BUNDLE_DEPLOYMENT_PATH/data
cp -v ../../data/walnut.raw dist/$APP_BUNDLE_DEPLOYMENT_PATH/data

# copy remaining data
mkdir dist/$APP_BUNDLE_DEPLOYMENT_PATH/data/networks
rsync -rv --exclude=.svn ../../data/networks/ dist/$APP_BUNDLE_DEPLOYMENT_PATH/data/networks/
mkdir dist/$APP_BUNDLE_DEPLOYMENT_PATH/data/workspaces
rsync -rv --exclude=.svn ../../data/workspaces/ dist/$APP_BUNDLE_DEPLOYMENT_PATH/data/workspaces/
mkdir dist/$APP_BUNDLE_DEPLOYMENT_PATH/data/networks/scripts
rsync -rv --exclude=.svn ../../data/scripts/ dist/$APP_BUNDLE_DEPLOYMENT_PATH/data/scripts/
mkdir dist/$APP_BUNDLE_DEPLOYMENT_PATH/data/networks/transferfuncs
rsync -rv --exclude=.svn ../../data/transferfuncs/ dist/$APP_BUNDLE_DEPLOYMENT_PATH/data/transferfuncs/

# create symbolic link to Application directory in disk image
echo ""
echo "* Generating symbolic link to Application directory ..."
ln -sv /Applications dist



# Create disk image
echo ""
echo "* ---------------------------------------------------"
echo "*  Creating $DMG_NAME.dmg from directory 'dist' ...  "
echo "* ---------------------------------------------------"
hdiutil create -volname $DMG_NAME -srcfolder ./dist/ ./$DMG_NAME.dmg


# Finally, print out dylib dependencies
echo ""
echo "* Application dylib dependencies: "
otool -L dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/MacOS/$APP_BINARY_NAME
echo ""
echo "* Voltool dylib dependencies: "
otool -L dist/$APP_BUNDLE_DEPLOYMENT_PATH/$APP_BUNDLE_NAME/Contents/Tools/$VOLTOOL_BINARY_NAME


# delete dist directory
#rm -rf dist

echo ""
echo "finished: $DMG_NAME.dmg"
echo ""
