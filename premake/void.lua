void = void or {}

local function join(base, rel)
    if base == nil or base == "" then
        return rel
    end
    return path.join(base, rel)
end

function void.set_common_project_settings(groups)
    r2.set_common_project_settings(groups)
end


function void.add_projects(opts, groups)
    local base     = opts.base or ""
    local r2_dir   = opts.r2_dir
    local backend  = opts.backend
    local build_root = opts.build_root
    local int_root   = opts.int_root

    -- resources
    project "resources"
        kind "StaticLib"
        targetname "%{prj.name}_%{cfg.buildcfg}_%{cfg.platform}"
        targetdir (build_root)
        objdir    (int_root)
        location  (join(base, "resources"))

        void.set_common_project_settings(groups)
        r2.set_project_backend_defines(backend)

        local shader_glob = join(base, "resources/res/shaders/**.hlsl")
        files {
            join(base, "resources/include/**.h"),
            join(base, "resources/internal/**.cpp"),
            join(base, "resources/internal/**.h"),
            shader_glob,
        }

        includedirs {
            join(base, "resources/ext"),
            join(base, "resources/include"),
        }

        -- hlsl preprocessing
        local pp_out = "%{prj.location}/../resources/res/shaders/out"
        filter { "files:" .. shader_glob }
            buildaction "CustomBuild"
            buildmessage "Preprocessing %{file.name}"

            if backend == "d3d11" then
                buildcommands {
                    'cl.exe /nologo /EP /DR2_BACKEND_D3D11 "%{file.abspath}" > "' .. pp_out .. '/%{file.basename}.shader"'
                }
            elseif backend == "opengl" then
                buildcommands {
                    'cl.exe /nologo /EP /DR2_BACKEND_OPENGL "%{file.abspath}" > "' .. pp_out .. '/%{file.basename}.shader"'
                }
            else
                error("void/resources: invalid backend.")
            end

            buildoutputs { pp_out .. "/%{file.basename}.shader" }
        filter {}

    -- void
    project "void"
        kind "StaticLib"
        targetname "%{prj.name}_%{cfg.buildcfg}_%{cfg.platform}"
        targetdir (build_root)
        objdir    (int_root)
        location  (join(base, "void"))

        void.set_common_project_settings(groups)
        r2.set_project_backend_defines(backend)

        files {
            join(base, "void/include/**.h"),
            join(base, "void/include/**.inl"),
            join(base, "void/src/**.h"),
            join(base, "void/src/**.cpp"),
        }

        includedirs {
            join(base, "void/include"),
            join(base, "void/ext"),
            join(base, "void/src"),
            join(base, "resources/include"),
            join(base, "resources/internal"),
        }

        r2.use { base = r2_dir, backend = backend, include_impl = true }

        dependson { "r2", "resources" }
        links     { "resources" }
end