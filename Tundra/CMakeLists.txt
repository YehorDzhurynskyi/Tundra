cmake_minimum_required(VERSION 3.1)

project(Tundra VERSION 1.0.0)

fbl_include_script(PrecompiledHeader.cmake)

set(Tundra_SRCS

    pch.h
    pch.cpp

    tundra.h
    tundra.cpp
)

add_library(Tundra STATIC ${Tundra_SRCS})
add_precompiled_header(Tundra pch.h SOURCE_CXX pch.cpp)

fbl_pack_sources(${Tundra_SRCS})

set_target_properties(Tundra
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
