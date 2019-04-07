#include "pch.h"
#include "Renderer.h"
#include "CCC/Camera.h"

#include "Layer/Event/Event.h"

#include <SDL_image.h>
#include <Fabula/Graphics/API/opengl.h>

namespace fbl
{

Renderer::Renderer()
    : m_currentSpriteCount(0)
{
    m_windowResizedListener.on(WindowResizedEvent::TypeID(), [this](const fbl::IEvent& event)
    {
        assert(event.GetEventTypeID() == WindowResizedEvent::TypeID());
        const WindowResizedEvent& windowResizedEvent = AS(const WindowResizedEvent&, event);

        FBL_GL_CALL(glBindTexture(GL_TEXTURE_2D, m_target_Texture));
        FBL_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowResizedEvent.Width, windowResizedEvent.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));

        return true;
    });
}

void Renderer::onConnect(fbl::Layer& layer)
{
    m_windowResizedListener.bind(layer);
}

bool Renderer::init()
{
    const bool staticPassSucceed = m_staticPass.init();
    const bool motionPassSucceed = m_motionBlurPass.init();

    assert(staticPassSucceed);
    assert(motionPassSucceed);

    bool initialized = staticPassSucceed && motionPassSucceed;
    if (!initialized)
    {
        return false;
    }

    { // Texture Atlas
        FBL_GL_CALL(glGenTextures(1, &m_atlas_Texture));
        FBL_GL_CALL(glBindTexture(GL_TEXTURE_2D, m_atlas_Texture));

        FBL_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        FBL_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        FBL_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        FBL_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        SDL_Surface* atlasSurface = IMG_Load("Assets/atlas.png");
        if (atlasSurface == nullptr)
        {
            REVEAL_SDL_ERROR("Failed to load sprite atlas")
        }

        FBL_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasSurface->w, atlasSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasSurface->pixels));
        FBL_GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

        SDL_FreeSurface(atlasSurface);
    }

    { // FBO & Target Texture
        FBL_GL_CALL(glGenFramebuffers(1, &m_FBO));
        FBL_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));

        FBL_GL_CALL(glGenTextures(1, &m_target_Texture));
        FBL_GL_CALL(glBindTexture(GL_TEXTURE_2D, m_target_Texture));

        const vec2f screenSize = Camera::get().getScreenSize();
        FBL_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenSize.x, screenSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));

        FBL_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        FBL_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        FBL_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_target_Texture, 0));

        initialized = FBL_GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        FBL_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    return initialized;
}

void Renderer::shutdown()
{
    FBL_GL_CALL(glDeleteFramebuffers(1, &m_FBO));

    FBL_GL_CALL(glDeleteTextures(1, &m_target_Texture));
    FBL_GL_CALL(glDeleteTextures(1, &m_atlas_Texture));

    m_staticPass.shutdown();
    m_motionBlurPass.shutdown();
}

void Renderer::render(const SpriteURI uri, const Transform& transform, const u32 colorTint)
{
    render(SpriteAtlas::at(uri).Offset, SpriteAtlas::at(uri).Size, transform, colorTint);
}

void Renderer::render(const AnimatedSpriteURI uri, const int frame, const Transform& transform, const u32 colorTint)
{
    const AnimatedSprite& sprite = SpriteAtlas::at(uri);

    assert(frame == clamp<i32>(frame, 0, sprite.NOfFrames - 1));
    assert(sprite.Pitch != 0);
    assert(sprite.NOfFrames / sprite.Pitch != 0);

    const float dU = sprite.Size.x / sprite.Pitch;
    const float dV = sprite.Size.y / (sprite.NOfFrames / sprite.Pitch);

    const float uOffset = sprite.Offset.x + dU * (frame % sprite.Pitch);
    const float vOffset = sprite.Offset.y + dV * (frame / sprite.Pitch);

    render(vec2f(uOffset, vOffset), vec2f(dU, dV), transform, colorTint);
}

void Renderer::render(const vec2f uvOffset, const vec2f uvSize, const Transform& transform, const u32 colorTint)
{
    const float sx = transform.Size.x;
    const float sy = transform.Size.y;

    Position_VBO.push(vec2f(-sx, sy) + transform.Position);
    Position_VBO.push(vec2f(-sx, -sy) + transform.Position);
    Position_VBO.push(vec2f(sx, sy) + transform.Position);
    Position_VBO.push(vec2f(sx, -sy) + transform.Position);

    {
        Color_UV_Data& color_uv = Color_UV_VBO.push();
        color_uv.UV = uvSize * vec2f(0.0f, 0.0f) + uvOffset;
        color_uv.ColorTint = colorTint;
    }

    {
        Color_UV_Data& color_uv = Color_UV_VBO.push();
        color_uv.UV = uvSize * vec2f(0.0f, 1.0f) + uvOffset;
        color_uv.ColorTint = colorTint;
    }

    {
        Color_UV_Data& color_uv = Color_UV_VBO.push();
        color_uv.UV = uvSize * vec2f(1.0f, 0.0f) + uvOffset;
        color_uv.ColorTint = colorTint;
    }

    {
        Color_UV_Data& color_uv = Color_UV_VBO.push();
        color_uv.UV = uvSize * vec2f(1.0f, 1.0f) + uvOffset;
        color_uv.ColorTint = colorTint;
    }

    const i32 offset = m_currentSpriteCount * 4;
    m_IBO.push(0 + offset);
    m_IBO.push(1 + offset);
    m_IBO.push(2 + offset);
    m_IBO.push(2 + offset);
    m_IBO.push(1 + offset);
    m_IBO.push(3 + offset);

    ++m_currentSpriteCount;
}

void Renderer::present_Before()
{
    assert(m_currentSpriteCount <= g_MaxVerticesCount / 4);

    {
        m_IBO.flush();
        Position_VBO.flush();
        Color_UV_VBO.flush();
    }
}

void Renderer::present_After()
{
    m_currentSpriteCount = 0;
}

}
