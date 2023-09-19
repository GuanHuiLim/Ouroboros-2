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

set FILE_NAME=ffx_spd_downsample_pass.glsl
"%VULKAN_SDK%\Bin\glslc.exe" --target-env=vulkan1.3 -DFFX_GLSL -DFFX_GPU -fshader-stage=comp -O  fidelity\src\backends\vk\shaders\spd\%FILE_NAME% -o bin/%FILE_NAME%.spv -I fidelity\include\FidelityFX\gpu"
 if !ERRORLEVEL! NEQ 0 (
			:: This "CMD" here is needed as a hack...
			CMD /C echo/
			echo [91m%FILE_NAME% [COMPILATION ERROR][0m
			pause
			exit /B 1
			echo.
		) else ( 
			:: This "CMD" here is needed as a hack...
			CMD /C echo/ 
			echo [92m%FILE_NAME%  [COMPILATION SUCCESS][0m
)


for %%i in (*.vert *.frag *.comp *.geom) do (
	
	set numberstring=%%~i
	set compile=1
	if !compile! EQU 1 ( 	
		"%VULKAN_SDK%\Bin\glslc.exe" --target-env=vulkan1.3 -std=460 -O "%%~i" -o "bin/%%~i.spv"
		if !ERRORLEVEL! NEQ 0 (
			:: This "CMD" here is needed as a hack...
			CMD /C echo/
			echo [91m%%~i [COMPILATION ERROR][0m
			pause
			exit /B 1
			echo.
		) else ( 
			:: This "CMD" here is needed as a hack...
			CMD /C echo/ 
			echo [92m!numberstring:~-28!  [COMPILATION SUCCESS][0m
		)
	)else (
		:: This "CMD" here is needed as a hack...
		CMD /C echo/ 
		echo [93m!numberstring:~-28! ^| no action[0m)
	)
	
	rem echo.
 )
 
 

popd
 echo.
 rem call touch done.txt
 echo Finished
 echo.

rem forfiles /s /m *.vert /c "cmd /c %VULKAN_SDK%/Bin/glslangValidator.exe -V @path -o @fname.vert.spv"
rem forfiles /s /m *.frag /c "cmd /c %VULKAN_SDK%/Bin/glslangValidator.exe -V @path -o @fname.frag.spv"
rem pause