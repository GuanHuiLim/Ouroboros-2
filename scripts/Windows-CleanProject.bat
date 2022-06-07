@echo off
pushd %~dp0\..\
rmdir /s /Q .vs
rmdir /s /Q bin
rmdir /s /Q bin-int
del /s /Q *.sln
popd
PAUSE