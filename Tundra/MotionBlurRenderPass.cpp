#include "pch.h"
#include "MotionBlurRenderPass.h"

#include <Fabula/Graphics/Renderer.h>
#include <Fabula/Layer/GameLayer.h>

namespace
{

const char* g_MotionBlurVertexShaderSource = ""
"attribute vec2 a_position;\n"
"\n"
"void main(void)\n"
"{\n"
"    gl_Position = vec4(a_position, 0.0, 1.0);\n"
"}\n";

const char* g_MotionBlurFragmentShaderSource = ""
#ifdef FBL_PLATFORM_WIN32
"#version 140\n"
#endif
"precision mediump float;\n"
"\n"
"uniform vec2 u_blurVec;\n"
"uniform vec2 u_blurVecOffset;\n"
"uniform vec2 u_screenSize;\n"
"\n"
"uniform sampler2D u_texture;\n"
"\n"
"void main(void)\n"
"{\n"
"    vec2 pixelUVCoord = gl_FragCoord.xy / u_screenSize;\n"
"    gl_FragColor = texture2D(u_texture, pixelUVCoord);\n"
"    int nOfSamples = 8;\n"
"    for (int i = 1; i < nOfSamples; ++i)\n"
"    {\n"
"        vec2 offset = u_blurVec * (float(i) / float(nOfSamples - 1)) - u_blurVecOffset;\n"
"        gl_FragColor += texture2D(u_texture, pixelUVCoord + offset);\n"
"    }\n"
"    gl_FragColor /= float(nOfSamples);\n"
"}\n";

}

bool MotionBlurRenderPass::init()
{
    const bool vertexShaderAttached = m_program.attachVertexShader(g_MotionBlurVertexShaderSource);
    const bool fragmentShaderAttached = m_program.attachFragmentShader(g_MotionBlurFragmentShaderSource);

    assert(vertexShaderAttached && fragmentShaderAttached);
    if (!vertexShaderAttached || !fragmentShaderAttached)
    {
        return false;
    }

    m_program.build();

    m_positionLocation = m_program.getAttributeLocation("a_position");

    m_blurVecLocation = m_program.getUniformLocation("u_blurVec");
    m_blurVecOffsetLocation = m_program.getUniformLocation("u_blurVecOffset");
    m_screenSizeLocation = m_program.getUniformLocation("u_screenSize");

    if (m_positionLocation < 0 ||
        m_blurVecLocation < 0 ||
        m_blurVecOffsetLocation < 0 ||
        m_screenSizeLocation < 0)
    {
        m_program.release();
        return false;
    }

    return true;
}

void MotionBlurRenderPass::shutdown()
{
    m_program.release();
}

void MotionBlurRenderPass::bind()
{
    {
        Renderer::get().Position_VBO.bind();
        FBL_GL_CALL(glVertexAttribPointer(m_positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(vec2f), (void*)0));
    }

    m_program.use();

    {
        const vec2f& currentPlayerPosition = g_Game->getPlayer().Transform.Position;
        const vec2f& screenSize = Camera::get().getScreenSize();

        const vec2f& blurVec = (currentPlayerPosition - m_prevPlayerPosition) * 0.08f;
        const vec2f& blurVecOffset = blurVec * 0.75f;

        FBL_GL_CALL(glUniform2f(m_blurVecLocation, -blurVec.x, blurVec.y));
        FBL_GL_CALL(glUniform2f(m_blurVecOffsetLocation, -blurVecOffset.x, blurVecOffset.y));
        FBL_GL_CALL(glUniform2f(m_screenSizeLocation, screenSize.x, screenSize.y));

        m_prevPlayerPosition = currentPlayerPosition;
    }

    FBL_GL_CALL(glEnableVertexAttribArray(m_positionLocation));
}

void MotionBlurRenderPass::unbind()
{
    FBL_GL_CALL(glDisableVertexAttribArray(m_positionLocation));
}
