cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0079 NEW)

add_subdirectory(Fabula)
add_subdirectory(Tundra)

target_link_libraries(Fabula
    Tundra
)
