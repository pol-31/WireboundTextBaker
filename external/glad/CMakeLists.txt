
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)
target_include_directories(glad PRIVATE ${CMAKE_SOURCE_DIR}/external/glad/include)
