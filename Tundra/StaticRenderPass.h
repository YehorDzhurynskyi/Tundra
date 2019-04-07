#pragma once

#include <Fabula/Graphics/Shader/ShaderProgram.h>

class StaticRenderPass
{
public:
    bool init();
    void shutdown();

    void bind();
    void unbind();

private:
    ShaderProgram m_program;

    ShaderLocationID m_positionLocation;
    ShaderLocationID m_uvLocation;
    ShaderLocationID m_colorTintLocation;
};
