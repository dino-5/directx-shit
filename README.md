# Build

1. Install all submodules(it is only sharpmake)
2. Open solution for sharpmake(to load some nuget packages)
3. If you want you can build it with VS, or go to **scripts** folder and run **sharpmake_build.bat**. It will build all data and move it to the correct destination
4. If you will build from VS you can go to Sharpmake/Sharpmake.Application/bin/Release/net6.0 delete all folders inside except runtimes, then delete everything from root folder and paste the remaining items of net6.0 folder
5. run **scripts/generate.bat**
