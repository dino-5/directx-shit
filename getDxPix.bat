powershell -command "rmdir source/include "
powershell -command "mkdir source/include"

rmdir /s /q bin
mkdir bin

cd generated/packages
rmdir /s /q AgilitySDK
mkdir AgilitySDK
powershell -command "Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.610.0 -OutFile agility.zip"
powershell -command "Expand-Archive agility.zip -DestinationPath AgilitySDK"
powershell -command "rm agility.zip"  
powershell -command "Move-Item AgilitySDK/build/native/include/ ../../source/include/dx12"
powershell -command "Move-Item AgilitySDK/build/native/bin/x64/* ../../bin/"

rmdir /s /q PIX
mkdir PIX
powershell -command "Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001 -OutFile pix.zip"
powershell -command "Expand-Archive pix.zip -DestinationPath PIX"
powershell -command "rm pix.zip"
powershell -command "Move-Item Pix/Include/WinPixEventRuntime ../../source/include/WinPix"
powershell -command "Move-Item Pix/bin/x64/* ../../bin/"

cd ../..

