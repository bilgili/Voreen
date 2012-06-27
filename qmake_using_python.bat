@echo off
rem Uncomment, if qt is not in your path
rem set path=c:\qt\4.4.3\bin

rem Select your Visual Studio version
rem set QMAKESPEC=win32-msvc.net
rem set QMAKESPEC=win32-msvc2005
set QMAKESPEC=win32-msvc2008

qmake.exe -tp vc -r
cd tools
call "project-deps.py"
cd ..