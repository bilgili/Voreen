@echo off
rem Uncomment, if qt is not in your path
rem set path=c:\qt\4.4.3\bin

rem Select your Visual Studio version
rem set QMAKESPEC=win32-msvc.net
rem set QMAKESPEC=win32-msvc2005
set QMAKESPEC=win32-msvc2008

rem call tools\svnversion.bat . include\voreen\core\svnversion.h

echo Processing voreenlib_core.pro ...
cd src\core
qmake.exe -tp vc "CONFIG+=flat" voreenlib_core.pro

echo Processing voreenlib_qt.pro ...
cd ..\qt
qmake.exe -tp vc "CONFIG+=flat" voreenlib_qt.pro

echo Processing voreenve.pro ...
cd ..\..\apps\voreenve
qmake.exe -tp vc "CONFIG+=flat" voreenve.pro

echo Processing voltool.pro ...
cd ..\..\apps\voltool
qmake.exe -tp vc "CONFIG+=flat" voltool.pro

cd ..\..

echo Finished.
pause
