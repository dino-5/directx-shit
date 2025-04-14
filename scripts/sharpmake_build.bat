@echo off
MSBuild.exe ..\Sharpmake\Sharpmake.sln /p:Configuration=Release
setlocal

set "KEEP_FOLDER=runtimes"

pushd ..\Sharpmake\
pushd Sharpmake.Application\bin\Release\net6.0\

for /d %%F in (*) do (
    if /i not "%%F"=="%KEEP_FOLDER%" (
        echo Deleting folder: %%F
        rmdir /s /q "%%F"
    )
)
popd 
set "KEEP_FOLDER=Sharpmake.Application"

for /d %%F in (*) do (
    if /i not "%%F"=="%KEEP_FOLDER%" (
        echo Deleting folder: %%F
        rmdir /s /q "%%F"
    )
)

:: Delete files (optional — remove if you want to keep files)
for %%F in (*) do (
    echo Deleting file: %%F
    del /f /q "%%F"
)

xcopy "Sharpmake.Application\bin\Release\net6.0\*" ".\" /E /I /Y

del /s /q Sharpmake.Application

pause
