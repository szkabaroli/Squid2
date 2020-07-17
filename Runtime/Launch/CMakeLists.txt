# Set up file variables 
file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_LIST_DIR}/Source/**)
file(GLOB_RECURSE PUBLIC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Public/**.h)

# == Common target ==========================================================
add_library(Squid INTERFACE)

target_include_directories(Squid INTERFACE .)
target_include_directories(Squid INTERFACE ${SDL2_INCLUDE_DIRS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(Squid INTERFACE ${SDL2_LIBRARIES})
target_link_libraries(Squid INTERFACE Threads::Threads)

target_link_libraries(Squid INTERFACE Core)

# TODO: dynamicly figure out
# We need the whole archive becouse the compiler strip out our unused code
target_link_libraries(Squid INTERFACE -Wl,--whole-archive VulkanRHI -Wl,--no-whole-archive) 
# target_link_libraries(${TARGET} -Wl,--whole-archive game -Wl,--no-whole-archive) 
# set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/WHOLEARCHIVE:vulkan_rhi")

# == Editor target ========================================================
add_executable(Squid-Editor ${SOURCE} ${PUBLIC_HEADERS})
target_link_libraries(Squid-Editor Squid)
target_link_libraries(Squid-Editor EditorCore-Editor)
target_link_libraries(Squid-Editor Renderer-Editor)
target_link_libraries(Squid-Editor imgui::imgui)
target_compile_definitions(Squid-Editor PRIVATE SQUID_EDITOR)
add_definitions(-DSQUID_EDITOR)

add_custom_command(
    TARGET Squid-Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:Squid-Editor>/Assets
    COMMENT "Copying editor assets" VERBATIM
)

# == Game target ==========================================================
add_executable(Squid-Game ${SOURCE} ${PUBLIC_HEADERS})
target_link_libraries(Squid-Game Squid)
#TODO: 
target_link_libraries(Squid-Game Renderer-Editor)

add_custom_command(
    TARGET Squid-Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:Squid-Game>/Assets
    COMMENT "Copying game assets" VERBATIM
)

# TODO: Server target