@echo off
SETLOCAL EnableDelayedExpansion

rem This sets this batch file's dir to itself
pushd %~dp0

set OUTPUT=bin\

echo [ CompilingShaders... ]
echo .
rem Create a "bin" folder if it does not exist, for shader binary output
if not exist %OUTPUT% (
  mkdir %OUTPUT%
)

set FID_PATH=fidelity\src\backends\vk\shaders

set "FSR2_FILES[0]=spd\ffx_spd_downsample_pass.glsl"
set "FSR2_FILES[1]=fsr2\ffx_fsr2_accumulate_pass.glsl" 
set "FSR2_FILES[2]=fsr2\ffx_fsr2_autogen_reactive_pass.glsl" 
set "FSR2_FILES[3]=fsr2\ffx_fsr2_compute_luminance_pyramid_pass.glsl"
set "FSR2_FILES[4]=fsr2\ffx_fsr2_depth_clip_pass.glsl"
set "FSR2_FILES[5]=fsr2\ffx_fsr2_lock_pass.glsl"
set "FSR2_FILES[6]=fsr2\ffx_fsr2_rcas_pass.glsl"
set "FSR2_FILES[7]=fsr2\ffx_fsr2_reconstruct_previous_depth_pass.glsl"
set "FSR2_FILES[8]=fsr2\ffx_fsr2_tcr_autogen_pass.glsl"

set submitCount=0

echo beginFSR
for /F "tokens=1* delims==" %%I in ('set FSR2_FILES[ 2^>nul') do (
	set /A submitCount+=1
	start /b cmd /c "echo Processing %%~nxJ && "%VULKAN_SDK%\Bin\glslc.exe" --target-env=vulkan1.3 -DFFX_GLSL -DFFX_GPU -fshader-stage=comp -O %FID_PATH%\%%J -o bin/%%~nxJ.spv -I fidelity\include\FidelityFX\gpu && (if !ERRORLEVEL! NEQ 0 (echo [91m%%~nxJ [COMPILATION ERROR][0m && set errorFlag=true) else (echo [92m%%~nxJ [COMPILATION SUCCESS][0m && echo|set /p="c">%%~nJ.chk))"
)
)
echo endFSR

echo normalFiles
for %%i in (*.vert *.frag *.comp *.geom) do (
	set /A submitCount+=1
	set fname=%%~i		
	start /b cmd /c ""%VULKAN_SDK%\Bin\glslc.exe" --target-env=vulkan1.3 -std=460 -O "%%~i" -o "bin/%%~i.spv" &&                                                                                                      (if !ERRORLEVEL! NEQ 0 (echo [91m%%~i [COMPILATION ERROR][0m && set errorFlag=true) else (echo [92m%%~i [COMPILATION SUCCESS][0m && echo|set /p="c">%%~i.chk))"
	rem echo.
 )
 
echo endNormal
 
:wait_loop
tasklist | find /i "cmd.exe" | find /i "glslc.exe" > nul
if not errorlevel 1 (
    ping localhost -n 2 > nul
    goto :wait_loop
)
echo All commands have finished.

set compiled=0
for %%i in (*.chk) do (
	set /A compiled+=1
	del %%i
 )
 

popd
 echo.
 rem call touch done.txt
 echo Finished
 echo.

if %compiled%==%submitCount% (
echo Compilation Success
exit /B 0
) else (
echo Compilation failure %compiled%/%submitCount%
exit /B 1
)

rem forfiles /s /m *.vert /c "cmd /c %VULKAN_SDK%/Bin/glslangValidator.exe -V @path -o @fname.vert.spv"
rem forfiles /s /m *.frag /c "cmd /c %VULKAN_SDK%/Bin/glslangValidator.exe -V @path -o @fname.frag.spv"
rem pause