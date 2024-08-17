require "vendor/premake/export-compile-commands/export-compile-commands"

platform_define = ""

if os.host() == "windows" then
    platform_define = "PLATFORM_WINDOWS"
elseif os.host() == "linux" then
    platform_define = "PLATFORM_LINUX"
elseif os.host() == "macosx" then
    platform_define = "PLATFORM_MAC"
end 

workspace "StimplyEngine"
    configurations { "Debug", "Release" }
    startproject "Stimply-Game"
   
project "Stimply-Engine"
    kind "SharedLib"
    language "C++"
    if os.host() == "windows" then
        cppdialect "c++17"
        libdirs { "vendor/SDL2" }
        links { "SDL2main", "SDL2", "vulkan-1" }
        flags { "MultiProcessorCompile" }
        defines { "DYNAMIC_RENDERER=__declspec(dllimport)", "RAPI=__declspec(dllexport)", "_CRT_SECURE_NO_WARNINGS" }
    elseif os.host() == "linux" then
        cppdialect "gnu++17"
        toolset "clang"
        defines { "DYNAMIC_RENDERER= ", "RAPI= ", "_XM_NO_XMVECTOR_OVERLOADS_" }
        libdirs { os.findlib("SDL2main") }
        libdirs { os.findlib("SDL2") }
        links { "SDL2main", "SDL2", "vulkan" }
        includedirs { "vendor/DirectXMath/Inc" }
        buildoptions {
            "-mavx2",
            "-mfma"
        }
    end
    targetdir "bin/%{cfg.buildcfg}"
       
    architecture("x86_64")
    files { "engine/**.cpp", "engine/**.h" }

    -- for some reason, $(VULKAN_SDK) wasn't working on linux, so using os.getenv and appending
    -- the include folder there.
    -- if VULKAN_SDK isn't set, this will return nil and error out.
    vulkan_sdk = os.getenv("VULKAN_SDK")
    includedirs { "engine/", "vendor/", vulkan_sdk .. "/include", "./" }

    -- defines for DirectXMath
    defines { "_XM_AVX2_INTRINSICS_", "_XM_AVX_INTRINSICS_", "_XM_SSE_INTRINSICS_", "_XM_SSE3_INTRINSICS_", "_XM_SSE4_INTRINSICS_", "_XM_FMA3_INTRINSICS_"  }

    filter "configurations:Debug"
        defines { "DEBUG", platform_define }
        debugdir "bin/Debug"
        symbols "On"

    filter "configurations:Release"
        defines { platform_define }
        debugdir "bin/Release"
        optimize "Full"

project "Stimply-Game"
    kind "ConsoleApp"
    language "C++"
    if os.host() == "windows" then
        cppdialect "c++17"
        defines { "RAPI=__declspec(dllimport)" }
        flags { "MultiProcessorCompile" }
    elseif os.host() == "linux" then
        defines { "RAPI= ", "_XM_NO_XMVECTOR_OVERLOADS_" }
        cppdialect "gnu++17"
        toolset "clang"
        includedirs { "vendor/DirectXMath/Inc" }
        buildoptions {
            "-mavx2",
            "-mfma"
        }
    end
    targetdir "bin/%{cfg.buildcfg}"

    architecture("x86_64")
    files { "game/**.cpp", "game/**.h" }

    links { "Stimply-Engine" }

    includedirs { "engine/" }
    
    -- defines for DirectXMath
    defines { "_XM_AVX2_INTRINSICS_", "_XM_AVX_INTRINSICS_", "_XM_SSE_INTRINSICS_", "_XM_SSE3_INTRINSICS_", "_XM_SSE4_INTRINSICS_", "_XM_FMA3_INTRINSICS_"  }

    filter "configurations:Debug"
        defines { "DEBUG", platform_define }
        debugdir "bin/Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { platform_define }
        debugdir "bin/Release"
        optimize "Full"