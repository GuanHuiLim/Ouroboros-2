@echo OFF 
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
echo "Starting "Debug Executable Build"
echo .  
devenv "../Ouroboros.sln" /Build "Debug|Executable"
echo . 
echo "build completed." 
pause