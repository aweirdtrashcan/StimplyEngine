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
   
project "Stimply-Engine"
    kind "SharedLib"
    language "C++"
    if os.host() == "windows" then
        cppdialect "c++17"
        libdirs { "vendor/SDL2" }
        links { "SDL2main", "SDL2" }
        flags { "MultiProcessorCompile" }
        defines { "DYNAMIC_RENDERER=__declspec(dllimport)", "RAPI=__declspec(dllexport)" }
    elseif os.host() == "linux" then
        cppdialect "gnu++17"
        toolset "clang"
        defines { "DYNAMIC_RENDERER= ", "RAPI= " }
        libdirs { os.findlib("SDL2main") }
        libdirs { os.findlib("SDL2") }
        links { "SDL2main", "SDL2" }
    end
    targetdir "bin/%{cfg.buildcfg}"
       
    architecture("x86_64")
    files { "engine/**.cpp", "engine/**.h" }

    includedirs { "engine/", "vendor/", "$(VULKAN_SDK)/include", "plugins/renderer" }

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
        defines { "RAPI= " }
        cppdialect "gnu++17"
        toolset "clang"
    end
    targetdir "bin/%{cfg.buildcfg}"

    architecture("x86_64")
    files { "game/**.cpp", "game/**.h" }

    links { "Stimply-Engine" }

    includedirs { "engine/", "vendor/" }

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
        links { "SDL2main", "SDL2", "vulkan-1" }
        flags { "MultiProcessorCompile" }
    elseif os.host() == "linux" then
        defines { "DYNAMIC_RENDERER= ", "RAPI= " }
        libdirs { os.findlib("SDL2main") }
        libdirs { os.findlib("SDL2") }
        links { "SDL2main", "SDL2" }
        cppdialect "gnu++17"
        toolset "clang"
    end
    targetdir "bin/%{cfg.buildcfg}"

    architecture("x86_64")
    files { "plugins/renderer/vulkan/**.cpp", "plugins/renderer/vulkan/**.h" }

    includedirs { "engine/", "$(VULKAN_SDK)/include", "vendor/" }

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
        links { "SDL2main", "SDL2", "d3d12" }
        flags { "MultiProcessorCompile" }
    elseif os.host() == "linux" then
        defines { "DYNAMIC_RENDERER= ", "RAPI= " }
        libdirs { os.findlib("SDL2main") }
        libdirs { os.findlib("SDL2") }
        links { "SDL2main", "SDL2" }
        cppdialect "gnu++17"
        toolset "clang"
    end
    targetdir "bin/%{cfg.buildcfg}"

    architecture("x86_64")
    files { "plugins/renderer/directx/**.cpp", "plugins/renderer/directx/**.h" }

    includedirs { "engine/", "vendor/" }

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