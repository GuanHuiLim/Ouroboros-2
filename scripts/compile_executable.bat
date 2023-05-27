@echo off
pushd %~dp0\..\bin\Production-Executable-windows-x86_64\Editor
echo "Cleaning executable file location"
call CleanUp-Exe.bat
echo "Cleaning Complete"
popd

echo "Building Exe"
"..\Editor\Inno Setup 6\ISCC.exe" /Qp ..\bin\Production-Executable-windows-x86_64\Editor\executable_portable.iss
echo "Build Complete"

PAUSE