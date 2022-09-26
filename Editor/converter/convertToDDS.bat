@echo off

pushd %~dp0

set INPUT=../assets/
set COMPRESSION=-dxt1
set OUTPUT=-outsamedir

crunch_x64 -file %INPUT%/* -deep -timestamp -ignoreerrors -fileformat dds %COMPRESSION% %OUTPUT%

popd
