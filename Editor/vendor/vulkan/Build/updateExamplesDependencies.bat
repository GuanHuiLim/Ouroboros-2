echo OFF
setlocal enabledelayedexpansion
set BUILD_DIR="%cd%"

rem --------------------------------------------------------------------------------------------------------
rem Set the color of the terminal to blue with yellow text
rem --------------------------------------------------------------------------------------------------------
COLOR 8E
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
powershell write-host -fore Cyan 	Updating dependancies
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
echo.

:DOWNLOAD_DEPENDENCIES
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
powershell write-host -fore White DOWNLOADING DEPENDENCIES
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
echo.

rem ------------------------------------------------------------
rem  GLM
rem ------------------------------------------------------------
:GLM
rmdir "../vendor/glm" /S /Q
git clone https://github.com/g-truc/glm.git "../vendor/glm"
if %ERRORLEVEL% GEQ 1 goto :ERROR
rem cd ../dependencies/xgeom_compiler/build
rem if %ERRORLEVEL% GEQ 1 goto :ERROR
rem call updateDependencies.bat "return"
rem if %ERRORLEVEL% GEQ 1 goto :ERROR
rem cd /d %BUILD_DIR%
rem if %ERRORLEVEL% GEQ 1 goto :ERROR

rem ------------------------------------------------------------
rem  GLM
rem ------------------------------------------------------------
:GLM
rmdir "../vendor/imgui" /S /Q
rem we want to clone docking branch
git clone -b docking https://github.com/ocornut/imgui.git "../vendor/imgui"
if %ERRORLEVEL% GEQ 1 goto :ERROR

rem ------------------------------------------------------------
rem  ASSIMP 
rem ------------------------------------------------------------
:ASSIMP
rmdir "../vendor/assimp" /S /Q
git clone https://github.com/assimp/assimp.git "../vendor/assimp"
if %ERRORLEVEL% GEQ 1 goto :ERROR

cd ../vendor/assimp
if %ERRORLEVEL% GEQ 1 goto :ERROR

cmake CMakeLists.txt -G "Visual Studio 16 2019" -S . -B ./BINARIES/Win32
if %ERRORLEVEL% GEQ 1 goto :ERROR

cmake --build ./BINARIES/Win32 --config release
if %ERRORLEVEL% GEQ 1 goto :ERROR

cd /d %BUILD_DIR%
if %ERRORLEVEL% GEQ 1 goto :ERROR



:DONE
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
powershell write-host -fore White DONE
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
goto :PAUSE

:ERROR
powershell write-host -fore Red ------------------------------------------------------------------------------------------------------
powershell write-host -fore Red  ERROR
powershell write-host -fore Red ------------------------------------------------------------------------------------------------------

:PAUSE
rem if no one give us any parameters then we will pause it at the end, else we are assuming that another batch file called us
if %1.==. pause

