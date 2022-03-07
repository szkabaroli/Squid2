cmake_minimum_required(VERSION 3.14)
project(squid VERSION 1.0)

set(STATIC_LINK_MODULES off)
set(CMAKE_UNITY_BUILD OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(UNITY_BUILD OFF)

set(CMAKE_UNITY_BUILD_BATCH_SIZE 4)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Bin)

if(STATIC_LINK_MODULES)
    add_definitions(-DSQUID_STATIC_LINK_MODULES)
endif()

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_latest_flag_supported)
    
    add_compile_options("/std:c++17")
    
endif()

if (WIN32)
    add_definitions(-DSQUID_WIN32)
    set(PLATFORM_WINDOWS ON)
elseif(UNIX AND NOT APPLE)
    add_definitions(-DSQUID_LINUX)
    set(PLATFORM_LINUX ON)
elseif(APPLE)
    add_definitions(-DSQUID_APPLE)
    set(PLATFORM_APPLE ON)
endif ()

# Find local SDL2 installation

set(CMAKE_PREFIX_PATH
    ${CMAKE_PREFIX_PATH}
    "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/SDL2-2.0.12"
)
set(SDL2_DIR "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/SDL2-2.0.12")
find_package(SDL2 REQUIRED)

# Third party libs
add_subdirectory("ThirdParty/vma")
add_subdirectory("ThirdParty/robin-map")
add_subdirectory("ThirdParty/glm")
add_subdirectory("ThirdParty/tinyobjloader")
add_subdirectory("ThirdParty/stb")
add_subdirectory("ThirdParty/ImGui")
add_subdirectory("ThirdParty/spdlog")
add_subdirectory("ThirdParty/half")

# Internal modules
add_subdirectory("Editor/EditorCore") # Module

add_subdirectory("Runtime/Core") # Always Static Module
add_subdirectory("Runtime/Launch") # Entry 
add_subdirectory("Runtime/RHI") # RHI (Interface lib) 
add_subdirectory("Runtime/VulkanRHI") # Module
add_subdirectory("Runtime/RenderGraph") # Module
add_subdirectory("Runtime/Renderer") # Module

# Game module
# add_subdirectory("Sandbox")