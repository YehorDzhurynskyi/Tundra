#include "pch.h"
#include "HUDLayer.h"

#include "GameLayer.h"
#include "Event/Event.h"

#include "Renderer.h"
#include <Fabula/Graphics/Text/TextRenderer.h>

HUDLayer::HUDLayer()
{
    m_clickEventListener.on(ClickEvent::TypeID(), [](const fbl::IEvent& event)
    {
        assert(event.GetEventTypeID() == ClickEvent::TypeID());

        const ClickEvent& clickEvent = AS(const ClickEvent&, event);
        if (clickEvent.NDCXPos > 0.9f &&
            clickEvent.NDCYPos < 0.05f)
        {
            SDL_Log("CLICK");
            return false;
        }

        return true;
    });

    connect(*this);
}

void HUDLayer::update()
{}

void HUDLayer::render() const
{
    {
        Transform t;
        t.Position = vec2f(0.9f, 0.9f);
        t.Size = vec2f(0.05f, 0.05f);

        Renderer::get().render(SpriteURI::IconPause, t, FBL_COLOR_WHITE);
    }

    {
        Renderer::get().present_Before();

        FBL_GL_CALL(glBindTexture(GL_TEXTURE_2D, Renderer::get().m_atlas_Texture));
        Renderer::get().m_staticPass.bind();
        FBL_GL_CALL(glDrawElements(GL_TRIANGLES, Renderer::get().m_currentSpriteCount * 6, GL_UNSIGNED_SHORT, (void*)0));
        Renderer::get().m_staticPass.unbind();

        Renderer::get().present_After();
    }

#if 0
    {
        char b[32];
        sprintf(b, "[22, 22, 22, ff]%i m", (i32)g_Game->m_player.DistanceCovered);
        TextRenderer::get().render_Text(b, vec2f(0.0f, 0.85f));
    }

    {
        char b[32];
        sprintf(b, "[0, 0, 0, ff]%i", (i32)(1.0f / g_DeltaTime));
        TextRenderer::get().render_Text(b, vec2f(-0.8f, 0.9f));
    }

#ifdef _DEBUG
    {
        char b[32];
        sprintf(b, "[ff, 22, 22, ff]%i", g_Game->m_player.BrakeParticles.Count);
        TextRenderer::get().render_Text(b, vec2f(-0.8f, 0.8f));
    }

    {
        char b[32];
        sprintf(b, "[ff, 22, ff, ff]%i", g_Game->m_player.TrailParticles.Count);
        TextRenderer::get().render_Text(b, vec2f(-0.8f, 0.7f));
    }

    {
        char b[32];
        sprintf(b, "[00, 63, 4a, ff]%i", g_Game->m_obstacles.Count);
        TextRenderer::get().render_Text(b, vec2f(-0.8f, 0.6f));
    }
#endif
#endif
    fbl::TextRenderer::get().render_Text("Hello", vec2f(0.0f, 0.0f));
    fbl::TextRenderer::get().present();
}

void HUDLayer::onConnect(Layer& layer)
{
    m_clickEventListener.bind(layer);
}
