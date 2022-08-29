@echo off

set OUTPUT=bin\

cd %OUTPUT%
for %%f in (*.spv) do (
	DEL %%f	
	echo deleted %%f;
)
cd "../"
echo.
echo done
pause