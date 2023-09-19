@ECHO OFF
SETLOCAL EnableDelayedExpansion

pushd %~dp0

for %%i in (*.vert *.frag *.comp *.geom) do (echo %%~i)

popd
pause