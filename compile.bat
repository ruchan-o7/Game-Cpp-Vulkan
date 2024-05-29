@echo off
cls
echo Building Shaders ...
cd Assets\Shaders
call compile.bat
if %errorlevel% NEQ 0 (pause)
echo Building Shaders DONE!
cd  ..\..\
echo Building Program ...
call cmake --build build -j
if %errorlevel% NEQ 0 (pause) else (echo Building Program DONE!)

