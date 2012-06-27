#!/bin/bash

echo ""
echo "Creating distribution disk image VoreenVE.dmg ..."
echo ""

# delete previous dmg
rm VoreenVE.dmg

#
# create temporary distribution directory and copy app bundle to it
#
mkdir dist
mkdir dist/VoreenVE
cp -R voreenve.app dist/VoreenVE/VoreenVE.app


#
# call Qt deployment tool on app bundle
#
macdeployqt dist/VoreenVE/VoreenVE.app


#
# copy external libs to app bundle and register them at the binary
#
mkdir dist/VoreenVE/VoreenVE.app/Contents/Library

cp /usr/lib/libGLEW.1.5.1.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/lib/libGLEW.1.5.1.dylib @executable_path/../Library/libGLEW.1.5.1.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/local/lib/libIL.1.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/local/lib/libIL.1.dylib @executable_path/../Library/libIL.1.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/local/lib/libILU.1.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/local/lib/libILU.1.dylib @executable_path/../Library/libILU.1.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/local/lib/libtiff.3.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/local/lib/libtiff.3.dylib @executable_path/../Library/libtiff.3.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/local/lib/libfreetype.6.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/local/lib/libfreetype.6.dylib @executable_path/../Library/libfreetype.6.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/local/lib/libftgl.2.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/local/lib/libftgl.2.dylib @executable_path/../Library/libftgl.2.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /System/Library/Frameworks/Python.framework/Versions/2.5/Python dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /System/Library/Frameworks/Python.framework/Versions/2.5/Python @executable_path/../Library/python dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/lib/libcrypto.0.9.7.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/lib/libcrypto.0.9.7.dylib @executable_path/../Library/libcrypto.0.9.7.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/lib/libssl.0.9.7.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/lib/libssl.0.9.7.dylib @executable_path/../Library/libssl.0.9.7.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/lib/libwrap.7.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/lib/libwrap.7.dylib @executable_path/../Library/libwrap.7.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/lib/libbz2.1.0.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/lib/libbz2.1.0.dylib @executable_path/../Library/libbz2.1.0.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

cp /usr/lib/libz.1.dylib dist/VoreenVE/VoreenVE.app/Contents/Library
install_name_tool -change /usr/lib/libz.1.dylib @executable_path/../Library/libz.1.dylib dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve

#
# copy voltool and register libraries
#
mkdir dist/VoreenVE/VoreenVE.app/Contents/Tools
cp ../voltool/voltool.app/Contents/MacOS/voltool dist/VoreenVE/VoreenVE.app/Contents/Tools

install_name_tool -change /usr/lib/libGLEW.1.5.1.dylib @executable_path/../Library/libGLEW.1.5.1.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/local/lib/libIL.1.dylib @executable_path/../Library/libIL.1.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/local/lib/libILU.1.dylib @executable_path/../Library/libILU.1.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/local/lib/libtiff.3.dylib @executable_path/../Library/libtiff.3.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/local/lib/libfreetype.6.dylib @executable_path/../Library/libfreetype.6.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/local/lib/libftgl.2.dylib @executable_path/../Library/libftgl.2.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /System/Library/Frameworks/Python.framework/Versions/2.5/Python @executable_path/../Library/python dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/lib/libcrypto.0.9.7.dylib @executable_path/../Library/libcrypto.0.9.7.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/lib/libssl.0.9.7.dylib @executable_path/../Library/libssl.0.9.7.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/lib/libwrap.7.dylib @executable_path/../Library/libwrap.7.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/lib/libbz2.1.0.dylib @executable_path/../Library/libbz2.1.0.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool

install_name_tool -change /usr/lib/libz.1.dylib @executable_path/../Library/libz.1.dylib dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool



#
# copy readonly resources to app bundle's resources path:
# - shaders.tar
# - textures
# - fonts
# - documentation 
#
cp ../../shaders.tar dist/VoreenVE/VoreenVE.app/Contents/Resources
cp -R ../../data/textures dist/VoreenVE/VoreenVE.app/Contents/Resources
cp -R ../../data/fonts dist/VoreenVE/VoreenVE.app/Contents/Resources
cp -R ../../doc dist/VoreenVE/VoreenVE.app/Contents/Resources

# copy over Info.plist and icon
cp ../../tools/macdeployment/Info.plist dist/VoreenVE/VoreenVE.app/Contents
cp ../../tools/macdeployment/icon.icns dist/VoreenVE/VoreenVE.app/Contents/Resources


#
# add user data
#
mkdir dist/VoreenVE/data

#copy volumes to data dir
cp ../../data/nucleon.dat dist/VoreenVE/data
cp ../../data/nucleon.raw dist/VoreenVE/data
cp ../../data/walnut.dat dist/VoreenVE/data
cp ../../data/walnut.raw dist/VoreenVE/data

# copy remaining data
cp -R ../../data/networks dist/VoreenVE/data
cp -R ../../data/workspaces dist/VoreenVE/data
cp -R ../../data/scripts dist/VoreenVE/data
cp -R ../../data/transferfuncs dist/VoreenVE/data

# create symbolic link to Application directory in disk image
ln -s /Applications dist

#
# create disk image
#
echo "Creating DMG ..."
hdiutil create -volname VoreenVE -srcfolder ./dist/ ./VoreenVE.dmg


#
# print out dylib dependencies
#
echo ""
echo "Application dylib dependencies: "
otool -L dist/VoreenVE/VoreenVE.app/Contents/MacOS/voreenve
echo ""
echo "Voltool dylib dependencies: "
otool -L dist/VoreenVE/VoreenVE.app/Contents/Tools/voltool


# delete dist directory
rm -rf dist

echo ""
echo "finished."