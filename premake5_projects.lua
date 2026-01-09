local function void_join(base_path, rel)
    local base = base_path or ""
    if base == "" then
        return rel
    end
    return path.join(base, rel)
end

function void_define_projects(base_path, build_root, int_root, r2_dir)
    local function P(rel) return void_join(base_path, rel) end
dofile(P(path.join(r2_dir, "premake5_common.lua")))
r2_define_common();

group "deps"
dofile(P(path.join(r2_dir, "premake5_projects.lua")))
r2_define_projects(r2_dir, build_root, int_root)
group ""

project "resources"
    kind "StaticLib"
    targetname "%{prj.name}_%{cfg.buildcfg}_%{cfg.platform}"
    targetdir (build_root)
    objdir    (int_root)
    location (P("resources"))
    
    files {
        P("resources/include/**.h"),
        P("resources/internal/**.cpp"),
        P("resources/internal/**.h"),
        P("resources/res/shaders/**.hlsl")
    }
    
    includedirs {
        P("resources/ext"),
        P("resources/include")
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
    location (P("void"))

    files {
        P("void/include/**.h"),
        P("void/include/**.inl"),
        P("void/src/**.h"),
        P("void/src/**.cpp")
    }

    includedirs {
        P("void/include"),
        P("void/ext"),
        P("void/src"),
        r2_dir .. "/r2/include",
        r2_dir .. "/backend/include",
        r2_dir .. "/backend_d3d11/include",
        P("resources/include"),
        P("resources/internal")
    }
    
    dependson { "r2", "resources" }
end