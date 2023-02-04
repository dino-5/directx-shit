workspace "DX12 Demo"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

    system "windows"
    systemversion "latest"
    architecture "x64"
    defines {"WIN32", "DEBUG"}
    


project "External"
    location "src/external"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir "build/%{cfg.buildcfg}"
    files {
        "src/external/**.cpp",
        "src/external/**.hpp",
        "src/external/**.h",
       }

    includedirs{
        "src/Framework",
        "src/",
        "src/external/imgui/",
        "src/external/"

    }
    removefiles{
        "**/examples/**"
    }

project "ShitEngine"
    location "src/Framework"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "build/%{cfg.buildcfg}"
    files{
        "src/Framework/**.cpp",
        "src/Framework/**.hpp",
        "src/Framework/**.h"
    }

    includedirs{
        "src/Framework",
        "src/",
        "src/external/imgui/",
        "src/external/"
    }



project "Demo"
    location "src/Demo"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    files{
        "src/Demo/**.cpp",
        "src/Demo/**.hpp",
        "src/Demo/**.h",
        "src/Demo/Shaders/**.hlsl",
        
    }
    includedirs{
        "src/Framework",
        "src/",
        "src/external/imgui/",
        "src/external/"
    }
    
    links
    {
        "Framework",
        "External"
    }

    targetdir "build/%{cfg.buildcfg}"
    filter "files:**.hlsl"
       buildaction "None"


    
