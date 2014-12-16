@echo off
SET platform="x86"
IF "%1"=="x64" SET platform="x64"
IF NOT EXIST "build\" ( mkdir build )
cd build/
IF NOT EXIST "%platform%\" ( mkdir %platform% )
cd %platform%/
qmake ../../Bit7z.pro
nmake
cd ../../
