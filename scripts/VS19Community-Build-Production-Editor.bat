@echo OFF 
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
echo "Starting Production Editor Build"
echo .  
devenv "../Ouroboros.sln" /Build "Production|Editor"
echo . 
echo "build completed." 
pause