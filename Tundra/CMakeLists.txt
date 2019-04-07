cmake_minimum_required(VERSION 3.1)

project(Tundra VERSION 1.0.0)

fbl_include_script(PrecompiledHeader.cmake)

set(Tundra_SRCS

    pch.h
    pch.cpp

    TundraApplication.h
    TundraApplication.cpp

    Level.cpp
    Level.h
    MotionBlurRenderPass.cpp
    MotionBlurRenderPass.h
    StaticRenderPass.cpp
    StaticRenderPass.h
    Player.cpp
    Player.h

    CCC/Camera.h
    CCC/Camera.cpp

    Layer/ApplicationLayer.cpp
    Layer/ApplicationLayer.h
    Layer/GameLayer.cpp
    Layer/GameLayer.h
    Layer/HUDLayer.cpp
    Layer/HUDLayer.h
    Layer/Event/Event.h
)

add_executable(Tundra ${Tundra_SRCS})
add_precompiled_header(Tundra pch.h SOURCE_CXX pch.cpp)

target_link_libraries(Tundra
    PRIVATE
        Fabula
)

target_include_directories(Tundra
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_compile_definitions(Tundra
    PRIVATE
        _USE_MATH_DEFINES
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
