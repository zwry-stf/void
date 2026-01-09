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
    
    if _OPTIONS["has-glfw"] then
        defines { "VOID_HAS_GLFW" }
    end
    
    local r2_dir = "ext/r2"
dofile(path.join(r2_dir, "premake5_common.lua"))
r2_define_common();

group "deps"
dofile(path.join(r2_dir, "premake5_projects.lua"))
r2_define_projects(r2_dir, build_root, int_root)
group ""

dofile("premake5_projects.lua")
void_define_projects(nil, build_root, int_root, r2_dir)
    
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

    links { "r2", "backend" }
    filter { "configurations:*_d3d11" }
        links { "backend_d3d11", "d3d11", "d3dcompiler" }
    filter { }
    
    filter { "configurations:*_opengl" }
        links { "backend_opengl", "opengl32" }
    filter { }