@echo off
rem Uncomment, if qt is not in your path
rem set path=C:\Qt\2010.04-vc\qt\bin

rem Select your Visual Studio version
rem set QMAKESPEC=win32-msvc2005
set QMAKESPEC=win32-msvc2008

rem call tools\svnversion.bat . include\voreen\core\svnversion.h

echo Processing voreenlib_core.pro ...
cd src\core
qmake.exe -tp vc "CONFIG+=flat" voreenlib_core.pro
IF ERRORLEVEL 1 GOTO :error
echo.

echo Processing voreenlib_qt.pro ...
cd ..\qt
qmake.exe -tp vc "CONFIG+=flat" voreenlib_qt.pro
IF ERRORLEVEL 1 GOTO :error
echo.

echo Processing voreenve.pro ...
cd ..\..\apps\voreenve
qmake.exe -tp vc "CONFIG+=flat" voreenve.pro
IF ERRORLEVEL 1 GOTO :error
echo.

echo Processing voltool.pro ...
cd ..\..\apps\voltool
qmake.exe -tp vc "CONFIG+=flat" voltool.pro
IF ERRORLEVEL 1 GOTO :error
echo.

cd ..\..

echo Finished.
echo.
pause
GOTO :EOF

:error
echo.
echo FAILED!
echo.
pause
