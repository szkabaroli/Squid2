# Set up file variables 
file(GLOB SOURCE ${CMAKE_CURRENT_LIST_DIR}/Source/**.cpp)
file(GLOB PRIVATE_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Source/**.h)
file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Public/**.h)

# Create a static lib using the files
add_library(rhi INTERFACE ${PRIVATE_HEADERS} ${PUBLIC_HEADERS})

# Set include directories
target_include_directories(rhi INTERFACE ${CMAKE_CURRENT_LIST_DIR}/.)
target_include_directories(rhi INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Public)

# Link static libs
target_link_libraries(rhi INTERFACE Core)
