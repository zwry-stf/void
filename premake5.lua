workspace "void"
    configurations { "Debug", "Release" }
    platforms { "x86", "x64" }

    filter "platforms:x86"
        architecture "x86"
    filter "platforms:x64"
        architecture "x86_64"
    filter {}

    newoption {
        trigger = "has-glfw",
        description = "Enable GLFW integration"
    }

    newoption {
        trigger     = "backend",
        value       = "API",
        description = "Rendering backend",
        allowed = {
            { "d3d11",  "Direct3D 11" },
            { "opengl", "OpenGL" }
        }
    }

    local build_root = "build/%{prj.name}"
    local int_root   = "build/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

    local action = _ACTION
    if action == nil or action == "" then action = "unknown_action" end
    local host = os.host() or "unknown_host"
    location ("out/" .. action .. "/" .. host)

    if _OPTIONS["has-glfw"] then
        defines { "VOID_HAS_GLFW" }
    end

local r2_dir = "ext/r2"

include(path.join(r2_dir, "premake/r2.lua"))

group "deps"
    r2.add_projects {
        base = r2_dir,
        build_root = build_root,
        int_root = int_root,
        backend = _OPTIONS["backend"],
    }
group ""

include "premake/void.lua"
void.add_projects {
    base = "",
    build_root = build_root,
    int_root = int_root,
    r2_dir = r2_dir,
    backend = _OPTIONS["backend"],
}

project "TestRun"
    kind "WindowedApp"
    targetname "%{prj.name}_%{cfg.buildcfg}_%{cfg.platform}"
    targetdir (build_root)
    objdir    (int_root)
    location "TestRun"

    files { "TestRun/**.h", "TestRun/**.cpp" }
    includedirs { "TestRun/ext", "void/include", "resources/include" }

    dependson { "void" }
    links { "void", "resources" }

    r2.set_common_project_settings()
    r2.use { base = r2_dir, backend = _OPTIONS["backend"], include_impl = true }
    r2.set_project_backend_defines( _OPTIONS["backend"] )

    filter { "system:windows", "platforms:x64" }
        links { "TestRun/ext/GLFW/windows/x64/glfw3" }
    filter { "system:windows", "platforms:x86" }
        links { "TestRun/ext/GLFW/windows/x86/glfw3" }
    filter {}

    filter { "options:backend=opengl", "system:windows", "platforms:x64" }
        links { "TestRun/ext/gl/windows/x64/glew32s" }
    filter { "options:backend=opengl", "system:windows", "platforms:x86" }
        links { "TestRun/ext/gl/windows/x86/glew32s" }
    filter {}