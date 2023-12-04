-- pbr/xmake.lua

-- main project
target("pbr")
    set_kind("binary")
    add_files("src/*.cpp")
    add_headerfiles("src/*.h")
    add_deps("base")
    set_rundir("src/.")
target_end()