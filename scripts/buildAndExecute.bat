@echo off
call .\scripts\vsbuild.bat
call .\scripts\execute.bat
exit /b
call .\scripts\build.bat
call .\scripts\vsbuild.bat
