cmake_minimum_required(VERSION 4.1)
project(Saint_Valentin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

add_executable(Saint_Valentin
        src/main.cpp
        src/main.h)
target_link_libraries(Saint_Valentin PRIVATE SDL3 SDL3_image SDL3_ttf)

add_custom_command(TARGET Saint_Valentin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:Saint_Valentin>/assets
)