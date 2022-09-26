@echo off
pushd %~dp0\..\
call vendor\premake\bin\premake5.exe vs2019
set shaderDir=Editor\vendor\vulkan\OO_Vulkan\shaders\
if EXIST "%shaderDIR%\_ForceCleanAndCompileAll.bat" (call "%shaderDIR%\_ForceCleanAndCompileAll.bat") else (echo Cant compile shaders)
popd
PAUSE