# Set up file variables 
file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_LIST_DIR}/Source/**)
file(GLOB_RECURSE PUBLIC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Public/**.h)

# == Editor target =================================================================
add_library(EditorCore-Editor STATIC ${SOURCE} ${PUBLIC_HEADERS})

target_include_directories(EditorCore-Editor PRIVATE .)
target_include_directories(EditorCore-Editor PUBLIC ./Public)

target_link_libraries(EditorCore-Editor Core)
target_link_libraries(EditorCore-Editor Renderer-Editor)
target_link_libraries(EditorCore-Editor imgui::imgui)


target_include_directories(EditorCore-Editor PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(EditorCore-Editor ${SDL2_LIBRARIES})

set_target_properties(EditorCore-Editor PROPERTIES UNITY_BUILD ON)