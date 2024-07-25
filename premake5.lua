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
        links { "SDL2main", "SDL2" }
        flags { "MultiProcessorCompile" }
        defines { "DYNAMIC_RENDERER=__declspec(dllimport)", "RAPI=__declspec(dllexport)", "_CRT_SECURE_NO_WARNINGS" }
    elseif os.host() == "linux" then
        cppdialect "gnu++17"
        toolset "clang"
        defines { "DYNAMIC_RENDERER= ", "RAPI= ", "_XM_NO_XMVECTOR_OVERLOADS_" }
        libdirs { os.findlib("SDL2main") }
        libdirs { os.findlib("SDL2") }
        links { "SDL2main", "SDL2" }
        includedirs { "vendor/DirectXMath/Inc" }
        buildoptions {
            "-mavx2",
            "-mfma"
        }
    end
    targetdir "bin/%{cfg.buildcfg}"
       
    architecture("x86_64")
    files { "engine/**.cpp", "engine/**.h" }

    --links { "Stimply-Game" }

    includedirs { "engine/", "vendor/", "$(VULKAN_SDK)/include", "./" }

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

project "Stimply-Renderer-Backend-Vulkan"
    kind "SharedLib"
    language "C++"
    if os.host() == "windows" then
        cppdialect "c++17"
        defines { "DYNAMIC_RENDERER=__declspec(dllexport)" }
        libdirs { "$(VULKAN_SDK)/Lib", "vendor/SDL2" }
        links { "SDL2main", "SDL2", "vulkan-1", "Stimply-Engine" }
        flags { "MultiProcessorCompile" }
        postbuildcommands {
            "PowerShell .\\compile_shaders.ps1"
        }
    elseif os.host() == "linux" then
        defines { "DYNAMIC_RENDERER= ", "RAPI= ", "_XM_NO_XMVECTOR_OVERLOADS_" }
        libdirs { os.findlib("SDL2main") }
        libdirs { os.findlib("SDL2") }
        libdirs { "$(VULKAN_SDK)/lib" }
        links { "SDL2main", "SDL2", "vulkan", "Stimply-Engine" }
        cppdialect "gnu++17"
        toolset "clang"
        includedirs { "vendor/DirectXMath/Inc" }
        postbuildcommands {
            "./compile_shaders.sh"
        }
        buildoptions {
            "-mavx2",
            "-mfma"
        }
    end
    targetdir "bin/%{cfg.buildcfg}"

    architecture("x86_64")
    files { "plugins/renderer/vulkan/*.cpp", "plugins/renderer/vulkan/*.h" }

    includedirs { "engine/", "$(VULKAN_SDK)/include", "vendor/", }

    -- defines for DirectXMath
    defines { "_XM_AVX2_INTRINSICS_", "_XM_AVX_INTRINSICS_", "_XM_SSE_INTRINSICS_", "_XM_SSE3_INTRINSICS_", "_XM_SSE4_INTRINSICS_", "_XM_FMA3_INTRINSICS_"  }

    filter "configurations:Debug"
        if os.host() == "windows" then
            defines { "RAPI=__declspec(dllimport)" }
        end
        defines { "DEBUG", platform_define }
        debugdir "bin/Debug"
        symbols "On"

    filter "configurations:Release"
        if os.host() == "windows" then
            defines { "RAPI=__declspec(dllimport)" }
        end
        defines { platform_define }
        debugdir "bin/Release"
        optimize "Full"

project "Stimply-Renderer-Backend-DX12"
    kind "SharedLib"
    language "C++"
    if os.host() == "windows" then
        cppdialect "c++17"
        defines { "DYNAMIC_RENDERER=__declspec(dllexport)" }
        libdirs { "vendor/SDL2" }
        links { "SDL2main", "SDL2", "d3d12", "Stimply-Engine" }
        flags { "MultiProcessorCompile" }
    elseif os.host() == "linux" then
        defines { "DYNAMIC_RENDERER= ", "RAPI= ", "_XM_NO_XMVECTOR_OVERLOADS_" }
        libdirs { os.findlib("SDL2main") }
        libdirs { os.findlib("SDL2") }
        links { "SDL2main", "SDL2" }
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
    files { "plugins/renderer/directx/*.cpp", "plugins/renderer/directx/*.h" }

    includedirs { "engine/", "vendor/" }

    -- defines for DirectXMath
    defines { "_XM_AVX2_INTRINSICS_", "_XM_AVX_INTRINSICS_", "_XM_SSE_INTRINSICS_", "_XM_SSE3_INTRINSICS_", "_XM_SSE4_INTRINSICS_", "_XM_FMA3_INTRINSICS_"  }

    filter "configurations:Debug"
        if os.host() == "windows" then
            defines { "RAPI=__declspec(dllimport)" }
        end
        defines { "DEBUG", platform_define }
        debugdir "bin/Debug"
        symbols "On"

    filter "configurations:Release"
        if os.host() == "windows" then
            defines { "RAPI=__declspec(dllimport)" }
        end
        defines { platform_define }
        debugdir "bin/Release"
        optimize "Full"