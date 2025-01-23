@echo off
REM Define project paths
set PROJECT_ROOT=%~dp0%..\
set SOURCE_DIR=%PROJECT_ROOT%source
set BUILD_DIR=%PROJECT_ROOT%build
set COMPILER=cl
set THIRD_PARTY_DIR=%SOURCE_DIR%\third_party
set INCLUDE_DIR=%SOURCE_DIR% 

set PACKAGE_DIR=%ROOT_DIR%\generated\packages

set DX12_DIR=%PACKAGE_DIR%\Microsoft.Direct3D.D3D12.1.611.2\build\native
set DX12_INCLUDE_DIR=%DX12_DIR%\include
set DX12_LIB_DIR=%DX12_DIR%\bin\x64

set DXC_DIR=%PACKAGE_DIR%\Microsoft.Direct3D.DXC.1.7.2308.12\build\native
set DXC_INCLUDE_DIR=%DXC_DIR%\include
set DXC_LIB_DIR=%DXC_DIR%\lib\x64


REM Ensure the build directory exists
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

REM Collect all source files
set FILES=
SETLOCAL ENABLEDELAYEDEXPANSION
for /r "%SOURCE_DIR%" %%f in (*.cpp *.c *.cc) do (
    set FILES=!FILES! %%f
)


call vcvars64.bat

REM Compile the project
echo Compiling project with clang++...
%COMPILER% /MP /EHsc /W4 /std:c++20 /I%INCLUDE_DIR% /I%THIRD_PARTY_DIR%\imgui /I%THIRD_PARTY_DIR%\fmt\include /I%THIRD_PARTY_DIR%^
    /I%DX12_INCLUDE_DIR% /I%DXC_INCLUDE_DIR% ^
    %FILES% /Fe"%BUILD_DIR%\dx12_project.exe" ^
    /link /LIBPATH:%DX12_LIB_DIR% /LIBPATH:%DXC_LIB_DIR% ^
    d3d12.lib dxgi.lib gdi32.lib dxcompiler.lib

REM Check for success
if %errorlevel% equ 0 (
    echo Build succeeded!
) else (
    echo Build failed. Check errors above.
)
del *.obj
