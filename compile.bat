@echo off
cls
echo Building Shaders ...
cd Assets\Shaders
call compile.bat
if %errorlevel% NEQ 0 (pause)
echo Building Shaders DONE!
cd  ..\..\
echo Building Program ...
call cmake --build build\Ninja
call cmake --build build\VS
if %errorlevel% NEQ 0 (pause) else (echo Building Program DONE!)

