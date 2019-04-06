cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0079 NEW)

project(Tundra VERSION 1.0.0)

add_subdirectory(Fabula)
add_subdirectory(Tundra)

target_link_libraries(Fabula
    Tundra
)