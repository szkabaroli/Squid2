# Set up variables
set(VULKAN_SDK $ENV{VULKAN_SDK})

# Set up file variables 
set(SOURCES 
    Source/Module.cpp
    Source/Device.cpp
    Source/DescriptorSet.cpp
    Source/CommandAllocator.cpp
    Source/FboCache.cpp
    Source/Pipeline.cpp
    Source/RenderTarget.cpp
    Source/Swapchain.cpp
    Source/Texture.cpp
    Source/Buffer.cpp
)

set(HEADERS 
    Source/Raw.h
    Source/Module.h
    Source/Device.h
    Source/DescriptorSet.h
    Source/CommandAllocator.h
    Source/FboCache.h
    Source/Pipeline.h
    Source/RenderTarget.h
    Source/Swapchain.h
    Source/Texture.h
    Source/Buffer.h
)

# Create a static lib using the files
if(STATIC_LINK_MODULES)
    add_library(VulkanRHI STATIC ${SOURCES} ${HEADERS})
else()
    add_library(VulkanRHI SHARED ${SOURCES} ${HEADERS})
endif()

find_package(Vulkan REQUIRED)

target_include_directories(VulkanRHI PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(VulkanRHI ${SDL2_LIBRARIES})

# Set include directories
target_include_directories(VulkanRHI PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(VulkanRHI PRIVATE ${VULKAN_SDK}/include)

target_link_libraries(VulkanRHI Vulkan::Vulkan)
target_link_libraries(VulkanRHI tsl::robin_map)
target_link_libraries(VulkanRHI vma)

target_link_libraries(VulkanRHI Core)
target_link_libraries(VulkanRHI rhi)
#set_target_properties(VulkanRHI PROPERTIES UNITY_BUILD ON)
