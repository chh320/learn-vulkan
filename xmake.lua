set_project("tinyShader")

set_warnings("all")
set_languages("c++17")

add_rules("mode.release", "mode.debug")

add_includedirs("3rdParty/include")

add_requires("shaderc", "vulkansdk", "glfw", "tinygltf", "glm", "imgui")
add_packages("shaderc", "vulkansdk", "glfw", "tinygltf", "glm", "imgui", {public = true})

includes("base");
add_subdirs("pbr");
add_subdirs("csm");