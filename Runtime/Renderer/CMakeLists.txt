# Set up file variables 
set(SOURCES 
    Source/Mesh.cpp
    Source/Module.cpp
)

set(HEADERS 
    Public/Renderer/Mesh.h
    Public/Renderer/Module.h
    Public/Renderer/TextureImporter.h
)


add_library(Renderer-Editor STATIC ${SOURCES} ${HEADERS})

target_include_directories(Renderer-Editor PRIVATE .)
target_include_directories(Renderer-Editor PUBLIC ./Public)

target_include_directories(Renderer-Editor PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(Renderer-Editor ${SDL2_LIBRARIES})

target_link_libraries(Renderer-Editor tinyobjloader)
target_link_libraries(Renderer-Editor imgui::imgui)
target_link_libraries(Renderer-Editor stb)
target_link_libraries(Renderer-Editor EditorCore-Editor)
target_link_libraries(Renderer-Editor glm::glm)
target_link_libraries(Renderer-Editor half::half)
target_link_libraries(Renderer-Editor Core)
target_link_libraries(Renderer-Editor rhi)
target_link_libraries(Renderer-Editor rendergraph)

set_target_properties(Renderer-Editor PROPERTIES UNITY_BUILD ON)

target_compile_options(Renderer-Editor PRIVATE $<$<BOOL:${MSVC}>:/arch:AVX2>)