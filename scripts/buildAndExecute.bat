@echo off
call .\scripts\vsbuild.bat
pushd .\source\Demo\
.\output\win64\debug\demo.exe
popd
exit /b

call .\scripts\build.bat
call .\scripts\execute.bat

call .\scripts\vsbuild.bat
pushd .\source\Demo
.\source\Demo\output\win64\debug\demo.exe
popd
