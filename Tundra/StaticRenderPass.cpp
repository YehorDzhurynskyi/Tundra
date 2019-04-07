#include "pch.h"
#include "StaticRenderPass.h"

#include <Fabula/Graphics/API/opengl.h>
#include <Fabula/Graphics/Renderer.h>

const char* g_StaticVertexShaderSource = ""
"attribute vec2 a_position;\n"
"attribute vec2 a_uvtex;\n"
"attribute vec4 a_color_tint;\n"
"\n"
"varying vec2 v_uvtex;\n"
"varying vec4 v_color_tint;\n"
"\n"
"void main(void)\n"
"{\n"
"    v_uvtex = a_uvtex;\n"
"    v_color_tint = a_color_tint;\n"
"    gl_Position = vec4(a_position, 0.0, 1.0);\n"
"}\n";

const char* g_StaticFragmentShaderSource = ""
#ifdef FBL_PLATFORM_WIN32
"#version 140\n"
#endif
"precision mediump float;\n"
"\n"
"uniform sampler2D u_texture;\n"
"\n"
"varying vec2 v_uvtex;\n"
"varying vec4 v_color_tint;\n"
"\n"
"void main(void)\n"
"{\n"
"    gl_FragColor = v_color_tint * texture2D(u_texture, v_uvtex);\n"
"}\n";

bool StaticRenderPass::init()
{
    const bool vertexShaderAttached = m_program.attachVertexShader(g_StaticVertexShaderSource);
    const bool fragmentShaderAttached = m_program.attachFragmentShader(g_StaticFragmentShaderSource);

    assert(vertexShaderAttached && fragmentShaderAttached);
    if (!vertexShaderAttached || !fragmentShaderAttached)
    {
        return false;
    }

    m_program.build();

    m_positionLocation = m_program.getAttributeLocation("a_position");
    m_uvLocation = m_program.getAttributeLocation("a_uvtex");
    m_colorTintLocation = m_program.getAttributeLocation("a_color_tint");

    if (m_positionLocation < 0 ||
        m_uvLocation < 0 ||
        m_colorTintLocation < 0)
    {
        m_program.release();
        return false;
    }

    return true;
}

void StaticRenderPass::shutdown()
{
    m_program.release();
}

void StaticRenderPass::bind()
{
    {
        Renderer::get().Position_VBO.bind();
        FBL_GL_CALL(glVertexAttribPointer(m_positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(vec2f), (void*)0));
    }

    {
        Renderer::get().Color_UV_VBO.bind();
        FBL_GL_CALL(glVertexAttribPointer(m_colorTintLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Renderer::Color_UV_Data), (void*)offsetof(Renderer::Color_UV_Data, ColorTint)));
        FBL_GL_CALL(glVertexAttribPointer(m_uvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Renderer::Color_UV_Data), (void*)offsetof(Renderer::Color_UV_Data, UV)));
    }

    m_program.use();

    FBL_GL_CALL(glEnableVertexAttribArray(m_positionLocation));
    FBL_GL_CALL(glEnableVertexAttribArray(m_colorTintLocation));
    FBL_GL_CALL(glEnableVertexAttribArray(m_uvLocation));
}

void StaticRenderPass::unbind()
{
    FBL_GL_CALL(glDisableVertexAttribArray(m_positionLocation));
    FBL_GL_CALL(glDisableVertexAttribArray(m_colorTintLocation));
    FBL_GL_CALL(glDisableVertexAttribArray(m_uvLocation));
}
