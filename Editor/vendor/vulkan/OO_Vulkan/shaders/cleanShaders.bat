@echo off

rem This sets this batch file's dir to itself
pushd %~dp0

set OUTPUT=bin\

cd %OUTPUT%
for %%f in (*.spv) do (
	DEL %%f	
	echo deleted %%f;
)
cd "../"
echo.
popd
echo done
