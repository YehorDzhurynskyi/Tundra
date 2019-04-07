cmake_minimum_required(VERSION 3.1)

project(Tundra VERSION 1.0.0)

fbl_include_script(PrecompiledHeader.cmake)

set(Tundra_SRCS

    pch.h
    pch.cpp

    Tundra.h
    Tundra.cpp
)

add_executable(Tundra ${Tundra_SRCS})
add_precompiled_header(Tundra pch.h SOURCE_CXX pch.cpp)

target_link_libraries(Tundra
    PRIVATE
        Fabula
)

fbl_pack_sources(${Tundra_SRCS})

set_target_properties(Tundra
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

if(EXISTS "${CMAKE_SOURCE_DIR}/Data")
    add_custom_command(TARGET Tundra POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Data"
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )
    fbl_status_message("[CUSTOM POST BUILD STEP] Move project `Data` into Tundra binary directory")
else()
    fbl_fatal_message("Project `Data` should be located here: ${CMAKE_SOURCE_DIR}/Data")
endif()
