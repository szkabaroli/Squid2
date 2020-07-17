# Set up file variables 
file(GLOB SOURCE ${CMAKE_CURRENT_LIST_DIR}/Source/**.cpp)
file(GLOB PRIVATE_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Source/**.h)
file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Public/**.h)

# Create a static lib using the files
add_library(rendergraph STATIC ${SOURCE} ${PRIVATE_HEADERS} ${PUBLIC_HEADERS})

# Set include directories
target_include_directories(rendergraph PRIVATE ${CMAKE_CURRENT_LIST_DIR}/.)
target_include_directories(rendergraph PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Public)

# Link static libs
target_link_libraries(rendergraph Core)
target_link_libraries(rendergraph rhi)
set_target_properties(rendergraph PROPERTIES UNITY_BUILD ON)