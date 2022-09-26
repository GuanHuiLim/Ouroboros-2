@echo off

rem This sets this batch file's dir to itself
pushd %~dp0

set OUTPUT=bin\

cd %OUTPUT%
for %%f in (*.spv) do (
	DEL %%f	1>nul
)
cd "../"

set OUTPUT=bin\
rem Create a "bin" folder if it does not exist, for shader binary output
if not exist %OUTPUT% (
  mkdir %OUTPUT%
)

echo [92m####################################################[0m
echo [92m# Compiling all shaders to SPIR-V binary format... #[91m

for %%i in (*.vert *.frag *.comp *.geom) do (
	"%VULKAN_SDK%\Bin\glslc.exe" --target-env=vulkan1.2 -std=460 -O "%%~i" -o "bin/%%~i.spv"
)

echo [92m# Shader Compilation Ended...                      #[0m
echo [92m####################################################[0m

popd
pause
