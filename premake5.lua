workspace "void"
    configurations { "Debug_d3d11", "Debug_opengl", "Release_d3d11", "Release_opengl" }
    platforms { "x86", "x64" }
    language "C++"
    cppdialect "C++23"
    staticruntime "Off"

    newoption {
        trigger = "has-glfw",
        description = "Enable GLFW integration"
    }

    filter "platforms:x86"
        architecture "x86"
    filter "platforms:x64"
        architecture "x86_64"
    filter {}
    
    local build_root = "build/%{prj.name}"
    local int_root   = "build/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

    local action = _ACTION
    if action == nil or action == "" then
        action = "unknown_action"
    end
    local host = os.host() or "unknown_host"
        
    location ("out/" .. action .. "/" .. host)
    
    flags { "MultiProcessorCompile" }
    warnings "Extra"
    fatalwarnings { "All" }

    filter "action:vs*"
        buildoptions { "/sdl" }
    filter {}
    
    filter "configurations:Debug_*"
        symbols "On"
        defines { "_DEBUG" }
    filter "configurations:Release_*"
        optimize "On"
        intrinsics "On"
        linktimeoptimization "On"
        defines { "NDEBUG" }

        filter "action:vs*"
            buildoptions { "/Gy" }
        filter {}
    filter {}
    
    -- platform
    filter "system:windows"
        systemversion "latest"
        defines { "R2_PLATFORM_WINDOWS" }
    filter { "system:windows", "platforms:x86" }
        defines { "R2_PLATFORM_WINDOWS_X86" }
    filter { "system:windows", "platforms:x64" }
        defines { "R2_PLATFORM_WINDOWS_X64" }
    filter {}
    
    -- backend
    filter { "configurations:*_d3d11" }
        defines { "R2_BACKEND_D3D11" }
    filter { "configurations:*_opengl" }
        defines { "R2_BACKEND_OPENGL" }
    filter {}
    
    if _OPTIONS["has-glfw"] then
        defines { "VOID_HAS_GLFW" }
    end
    
    local r2_dir = "ext/r2"
group "deps"
dofile(path.join(r2_dir, "premake5_projects.lua"))
r2_define_projects(r2_dir, build_root, int_root)
group ""
    
project "resources"
    kind "StaticLib"
    targetname "%{prj.name}_%{cfg.buildcfg}_%{cfg.platform}"
    targetdir (build_root)
    objdir    (int_root)
    location "resources"
    
    files {
        "resources/include/**.h",
        "resources/internal/**.cpp",
        "resources/internal/**.h",
        "resources/res/shaders/**.hlsl"
    }
    
    includedirs {
        "resources/ext",
        "resources/include"
    }
    
    -- hlsl files
    local pp_out = "%{prj.location}/../resources/res/shaders/out"
    filter { "files:resources/res/shaders/**.hlsl", "configurations:*_d3d11" }
        buildaction "CustomBuild"
        buildmessage "Preprocessing %{file.name}"
        buildcommands {
            'cl.exe /nologo /EP /DR2_BACKEND_D3D11 ' ..
            '"%{file.abspath}" > "' .. pp_out .. '/%{file.basename}.shader"'
        }
        buildoutputs { pp_out .. "/%{file.basename}.shader" }
    filter { "files:resources/res/shaders/**.hlsl", "configurations:*_opengl" }
        buildaction "CustomBuild"
        buildmessage "Preprocessing %{file.name}"
        buildcommands {
            'cl.exe /nologo /EP /DR2_BACKEND_OPENGL ' ..
            '"%{file.abspath}" > "' .. pp_out .. '/%{file.basename}.shader"'
        }
        buildoutputs { pp_out .. "/%{file.basename}.shader" }
    filter {}
    
project "void"
    kind "StaticLib"
    targetname "%{prj.name}_%{cfg.buildcfg}_%{cfg.platform}"
    targetdir (build_root)
    objdir    (int_root)
    location "void"

    files {
        "void/include/**.h",
        "void/include/**.inl",
        "void/src/**.h",
        "void/src/**.cpp"
    }

    includedirs {
        "void/include",
        "void/ext",
        "void/src",
        r2_dir .. "/r2/include",
        r2_dir .. "/backend/include",
        r2_dir .. "/backend_d3d11/include",
        "resources/include",
        "resources/internal"
    }
    
    dependson { "r2", "resources" }

    
project "TestRun"
    kind "WindowedApp"
    targetname "%{prj.name}_%{cfg.buildcfg}_%{cfg.platform}"
    targetdir (build_root)
    objdir    (int_root)
    location "TestRun"

    files {
        "TestRun/**.h",
        "TestRun/**.cpp"
    }

    includedirs {
        "TestRun/ext",
        "void/include",
        r2_dir .. "/r2/include",
        r2_dir .. "/backend/include",
        r2_dir .. "/backend_d3d11/include",
        "resources/include"
    }

    dependson { "void" }
    links     { "void", "resources" }

    filter { "system:windows", "platforms:x64" }
        links { "TestRun/ext/GLFW/windows/x64/glfw3" }
    filter { "system:windows", "platforms:x86" }
        links { "TestRun/ext/GLFW/windows/x86/glfw3" }
    filter { }
    
    filter { "configurations:*_opengl", "system:windows", "platforms:x64" }
        links { "TestRun/ext/gl/windows/x64/glew32s" }
    filter { "configurations:*_opengl", "system:windows", "platforms:x86" }
        links { "TestRun/ext/gl/windows/x86/glew32s" }
    filter { }

    links     { "r2", "backend" }
    filter { "configurations:*_d3d11" }
        links { "backend_d3d11", "d3d11", "d3dcompiler" }
    filter { }
    
    filter { "configurations:*_opengl" }
        links { "backend_opengl", "opengl32" }
    filter { }