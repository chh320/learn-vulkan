target("base")
    set_kind("static")

    add_includedirs("./", {public = true})
    
    add_files("Assets/*.cpp")
    add_files("Utilities/*.cpp")
    add_files("Vulkan/*.cpp")
target_end()