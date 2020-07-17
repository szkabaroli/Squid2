# Set up file variables 
set(SOURCES 
    Source/ECS/Scene.cpp
    
    Source/Modules/EngineContext.cpp
    Source/Modules/ModuleManager.cpp
    
    Source/Random.cpp
    Source/Profiling.cpp
    Source/FileWatcher.cpp
    Source/Log.cpp
)

set(HEADERS 
    Public/Core/ECS/ComponentRegistry.h
    Public/Core/ECS/Entity.h
    Public/Core/ECS/HierarchyComponent.h
    Public/Core/ECS/Scene.h
    Public/Core/ECS/TransformComponent.h
    
    Public/Core/Modules/EngineContext.h
    Public/Core/Modules/IModule.h
    Public/Core/Modules/ModuleManager.h

    Public/Core/FileSystem.h
    Public/Core/FileDialog.h
    Public/Core/FileWatcher.h
    Public/Core/Log.h
    Public/Core/Murmur.h
    Public/Core/Profiling.h
    Public/Core/Random.h
    Public/Core/RingBuffer.h
    Public/Core/Types.h
)

if(PLATFORM_WINDOWS)
    
elseif(PLATFORM_LINUX)
    
endif()

# Create the exe
add_library(Core STATIC ${SOURCES} ${HEADERS})

target_include_directories(Core PRIVATE .)
target_include_directories(Core PUBLIC Public)

target_link_libraries(Core glm::glm)
target_link_libraries(Core spdlog)
target_link_libraries(Core ${CMAKE_DL_LIBS})

set_target_properties(Core PROPERTIES UNITY_BUILD OFF)

# target_precompile_headers(core PUBLIC ${CMAKE_CURRENT_LIST_DIR}/pch.h)