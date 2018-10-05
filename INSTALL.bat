@echo off

set old_dir=%cd%

cd %~dp0

REM delete build directory
rd /s /q build
del /f /q multiverse.exe

REM generate template
python script/generator.py

REM create build directory
mkdir build
if "%errorlevel%"=="1" goto :end

REM go to build
cd build
if "%errorlevel%"=="1" goto :end

REM cmake
cmake .. -G "MinGW Makefiles"
if "%errorlevel%"=="1" goto :end

REM make
mingw32-make
if "%errorlevel%"=="1" goto :end

REM install
copy src\multiverse.exe ..\

:end
cd %old_dir%