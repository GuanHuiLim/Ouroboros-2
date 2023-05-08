@echo OFF 
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
echo "Starting Production Editor Build"
echo .  
IF EXIST "../Editor\icons\Ouroboros.ico"\ (xcopy /Q /E /Y /I "../Editor\icons\Ouroboros.ico" "../Editor\icons\FinalIcon.ico" > nul) ELSE (xcopy /Q /Y /I "../Editor\icons\Ouroboros.ico" "../Editor\icons\FinalIcon.ico" > nul)
devenv "../Ouroboros.sln" /Build "Production|Editor"
echo . 
echo "build completed." 
pause