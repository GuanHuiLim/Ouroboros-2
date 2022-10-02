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

for %%i in (*.vert *.frag *.comp *.geom) do (
	rem this is full accuracy
	rem forfiles /M "%%~i.spv" /C "cmd /c set time=@ftime set date=@fdate 
	rem echo !time!
	rem echo !date!
	rem echo %%~ti
	for /f "tokens=1-10 delims=:/,. " %%A in ("%%~ti") do (
	 rem hacky fix for integer stuff replacement
	set /A "Day=10000%%A %% 10000"
	set /A "Month=10000%%B %% 10000"
	set /A "Year=10000%%C %% 10000"
	set /A "Hour=10000%%D %% 10000"
	set /A "Min=10000%%E %% 10000"
	set "f=%%F"
	if !f! == pm if !Hour! neq 12 ( set /A Hour=!Hour!+12 )
	)
	
	 rem echo day !Day!
	 rem echo month !Month!
	 rem echo year !Year!	
	 rem echo hours !Hour!
	 rem echo min !Min!	 
	 rem echo ampm !f!
	set /A compile=0
	for %%j in ("%OUTPUT%%%~i.spv") do (
	 IF exist "%%~j" (
				rem echo %%~tj				
				for /f "tokens=1-10 delims=:/,. " %%A in ("%%~tj") do (
				set /A "jDay=10000%%A %% 10000"
				set /A "jMonth=10000%%B %% 10000"
				set /A "jYear=10000%%C %% 10000"
				set /A "jHour=10000%%D %% 10000"
				set /A "jMin=10000%%E %% 10000"
				set "f=%%F"
				if !f! == pm if !jHour! neq 12 ( set /A jHour=!jHour!+12 )
				)
				rem echo jyear !jYear!	
				rem echo jmonth !jMonth!
				rem echo jday !jDay!
				rem echo jhours !jHour!
				rem echo jmin !jMin!
				rem echo ampm !f!
				
				set /A compile=0
				
				set /A "source=!Year! * 365 + !Month! * 30 + !Day!"
				set /A "output=!jYear! * 365 + !jMonth! * 30 +!jDay!"
				rem echo !source! comp1 !output!
				IF !source! GTR !output! (set /A compile=1) else (
					IF !source! EQU !output! (set /A "source=!Hour!*60+!Min!"
					set /A "output=!jHour!*60+!jMin!"
					rem echo !source! comp2 !output!
					IF !source! GEQ !output! (set /A compile=1))
					
				)
				rem IF !Year! GTR !jYear! (set /A compile=1) 
				rem IF !Month! GTR !jMonth! (set /A compile=1) 
				rem IF !Day! GTR !jDay! (set /A compile=1) 
				rem IF !Hour! GTR !jHour! (set /A compile=1) 
				rem IF !Min! GEQ !jMin! (set /A compile=1) 
							
				rem IF !Year! GTR !jYear! (set /A compile=1
				rem 						echo !Year! y greater than !jYear!) else (echo y lesser
				rem )
				rem 	IF !Month! GTR !jMonth! (set /A compile=1
				rem 	echo !Month! m greater than !jMonth!) else (echo m lesser
				rem 	)
				rem 		IF !Day! GTR !jDay! (set /A compile=1
				rem 		echo !Day! d greater than !jDay!) else (echo d lesser
				rem 		)
				rem 			IF !Hour! GTR !jHoufr! (set /A compile=1
				rem 			echo !Hour! h greater than !jHour!) else (echo h lesser
				rem 			)
				rem 				IF !Min! GEQ !jMin! (set /A compile=1
				rem 				echo !Min! m greater or same !jMin!) else (echo m lesser)
				
		) else (set /A compile=1
		rem does not exist
		)
		
	set numberstring=                      %%~i
	if !compile! EQU 1 ( 	
		"%VULKAN_SDK%\Bin\glslc.exe" --target-env=vulkan1.2 -std=460 -O "%%~i" -o "bin/%%~i.spv"
		if !ERRORLEVEL! NEQ 0 (
			:: This "CMD" here is needed as a hack...
			CMD /C echo/
			echo [91m%%~i [COMPILATION ERROR][0m
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