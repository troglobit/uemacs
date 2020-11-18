@echo off

REM Figure out the platform we're on, and use that as target platform
REM If the user wants another platform, set environment variable PLATFORM
REM to a desired value (x86, x64, ia64)
if "%PLATFORM%" equ "" (
    if "%PROCESSOR_ARCHITECTURE%" equ "x86" (
        if "%PROCESSOR_ARCHITEW6432%" neq "" (
            set PLATFORM=x64
        ) else (
            set PLATFORM=Win32
        )
    ) else (
        set PLATFORM=x64
    )
)

REM Assume the user wants Release (full optimization, no debug information)
REM Override by setting the environment variable CONFIGURATION to one of
REM Release or Debug.
if "%CONFIGURATION%" equ "" (
    set CONFIGURATION=Release
)

REM Check to make sure msbuild is available
WHERE /q msbuild
IF %ERRORLEVEL% NEQ 0 (
    ECHO msbuild not found on path. Try running this command in the Visual Studio developer command prompt.
    exit /b 1
)

pushd src\sys\windows
msbuild uemacs.vcxproj /p:Configuration=Release /p:Platfom=%PLATFORM% /p:OutDir=..\..\..\
popd

REM Check if the compilation failed
IF %ERRORLEVEL% NEQ 0 (
    ECHO msbuild failed. Have you installed the Visual Studio C++ workload? Examine the msbuild output above for more information.
    exit /b 1
)

REM Remove debug files the user isn't concerned with anyhow
del /q uemacs.iobj uemacs.ipdb uemacs.pdb