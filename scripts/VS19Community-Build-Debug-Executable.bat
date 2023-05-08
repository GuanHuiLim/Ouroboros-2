@echo OFF 
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
echo "Starting "Debug Executable Build"
echo .  
IF EXIST "../Editor\icons\Minute.ico"\ (xcopy /Q /E /Y /I "../Editor\icons\Minute.ico" "../Editor\icons\FinalIcon.ico" > nul) ELSE (xcopy /Q /Y /I "../Editor\icons\Minute.ico" "../Editor\icons\FinalIcon.ico" > nul)
devenv "../Ouroboros.sln" /Build "Debug|Executable"
echo . 
echo "build completed." 
pause