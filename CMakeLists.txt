cmake_minimum_required(VERSION 3.28)
project(WireboundTextBaker)

cmake_policy(SET CMP0069 NEW) # LTO support
cmake_policy(SET CMP0091 NEW) # MSVC runtime support
cmake_policy(SET CMP0075 NEW) # honor to CMAKE_REQUIRED_LIBRARIES

# for WireboundTextBaker/external/
cmake_policy(SET CMP0077 NEW) # honors normal variables

set(CMAKE_WARN_DEPRECATED OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external EXCLUDE_FROM_ALL)

add_executable(WireboundTextBaker
        main.cpp
        Parsing.cpp
        Renderer.cpp
        Serializing.cpp
        Shader.cpp
        Texture.cpp
)

set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

target_include_directories(WireboundTextBaker
        PUBLIC ${EXTERNAL_DIR}/glad/include
        PUBLIC ${EXTERNAL_DIR}/glfw/include
        PUBLIC ${EXTERNAL_DIR}/utfcpp
        PUBLIC ${EXTERNAL_DIR}/stb
)

target_link_libraries(WireboundTextBaker
        PUBLIC glad
        PUBLIC glfw
        PUBLIC stb
)

# c++23 installed here on my pc; you better to replace or just remove
target_include_directories(WireboundTextBaker
        PUBLIC /usr/local/include/c++/14.1.0
)
target_link_directories(WireboundTextBaker
        PUBLIC /usr/local/lib/gcc/x86_64-pc-linux-gnu/14.1.0
)

set_target_properties(WireboundTextBaker PROPERTIES
        BUILD_RPATH "/usr/local/lib64"
        INSTALL_RPATH "/usr/local/lib64"
)
