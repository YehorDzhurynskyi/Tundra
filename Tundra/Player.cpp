#include "pch.h"
#include "Player.h"

#include "Layer/GameLayer.h"
#include "Layer/Event/Event.h"

#include "Renderer.h"

#include <glm/glm.hpp>

namespace
{

const float g_SkewAngle = 30.0f;
const vec2f g_GravityForce = vec2f(0.0f, 8.0f);
const vec2f g_ConstantForce = sinf(to_radians(g_SkewAngle)) * g_GravityForce;

float foo(float x)
{
    return std::pow(M_E, 4.0f * (x - 1.0f));
}

}

Player::Player()
{
    m_directionSwitchListener.on(ClickEvent::TypeID(), [this](const fbl::IEvent& event)
    {
        assert(event.GetEventTypeID() == ClickEvent::TypeID());

        const ClickEvent& clickEvent = AS(const ClickEvent&, event);
        m_inertia = m_ownVelocity;
        m_ownVelocity.x = -1.0f * m_ownVelocity.x;
        m_inertiaDamping = 1.0f - m_inertiaDamping;

        return false;
    });
}

void Player::onConnect(fbl::Layer& layer)
{
    m_directionSwitchListener.bind(layer);
}

void Player::update()
{
    m_velocity = m_inertiaDamping * m_inertia + (1.0f - m_inertiaDamping) * m_ownVelocity;
    const vec2f& dp = g_DeltaTime * (g_ConstantForce + m_velocity);
    Transform.Position += dp;
    DistanceCovered += dp.length();

#if 0
    const float desiredZoom = m_inertiaDamping > 0.4f ? 1.4f : 1.0f;
    Camera::get().Zoom += (desiredZoom - Camera::get().Zoom) * 0.05f / (g_DeltaTime / 0.0166666f);
#endif

    m_inertiaDamping *= m_friction / (g_DeltaTime / 0.0166666f);
    if (m_inertiaDamping < 0.0025f)
    {
        m_inertiaDamping = 0.0f;
    }

    m_inertiaDamping = clamp<float>(m_inertiaDamping, 0.0f, 1.0f);

    const AnimatedSprite& sprite = SpriteAtlas::at(AnimatedSpriteURI::Player);
    m_currentFrame = m_inertiaDamping * (sprite.NOfFrames - 1);
    if (m_ownVelocity.x > 0.0f)
    {
        m_currentFrame = sprite.NOfFrames - 1 - m_currentFrame;
    }

    const float worldLimit = (GameLayer::g_MapWidth - Transform.Size.x) * 0.5f;
    Transform.Position.x = clamp<float>(Transform.Position.x, -worldLimit, worldLimit);

    update_Trail();
    update_Brake();
}

void Player::update_Trail()
{
    {
        static float timer;
        timer -= g_DeltaTime;

        if (timer <= 0.0f)
        {
            m_playerTrailBuffer.push(vec2f(Transform.Position.x, Transform.Position.y + 0.5f));
            FOR(5)
            {
                Particle* particle = TrailParticles.push();

                float offsetx = (2.0f * rand01() - 1.0f) * 0.3f;
                float offsety = rand01() * -2.0f;

                u8 r = (u8)(rand() % 255);
                u8 g = (u8)(rand() % 255);
                u8 b = (u8)(rand() % 255);

                particle->Transform.Position = Transform.Position + vec2f(offsetx, offsety);
                particle->ColorTint = FBL_COLOR(r, g, b, 0xff);
                particle->Life = 1.0f;
                particle->Velocity = -m_velocity * 0.015f * rand01();
            }
            timer = 0.1f;
        }
    }

    for (auto& node : TrailParticles)
    {
        if (!node.InUse)
        {
            continue;
        }

        Particle& particle = node.Value;
        particle.Life -= g_DeltaTime * 2.0f;

        particle.Transform.Position += particle.Velocity * g_DeltaTime;
        particle.Transform.Size = vec2f(0.25f, 0.25f) * particle.Life + vec2f(0.05f, 0.05f);

        const u8 alpha = particle.Life * 255;
        particle.ColorTint &= 0x00ffffff;
        particle.ColorTint |= (alpha << 24);
    }

    TrailParticles.rescan();
}

void Player::update_Brake()
{
    {
        static float timer;
        timer -= g_DeltaTime;
        if (timer < 0.0f)
        {
            Particle* particle = BrakeParticles.push();

            const float offsetx = sign(m_inertia.x) * (rand01() * 0.45f + 0.05f);
            const float offsety = rand01() * -0.35f + 0.05f;

            const u8 chnl = 0x66 * rand01();

            particle->Transform.Position = Transform.Position + vec2f(offsetx, offsety);
            particle->ColorTint = FBL_COLOR(0x88 + chnl, 0x88 + chnl, 0x88 + chnl, 0xff);
            particle->Life = 1.0f * m_inertiaDamping;
            particle->Velocity = m_inertia;
            timer = 0.01f;
        }
    }

    for (auto& node : BrakeParticles)
    {
        if (!node.InUse)
        {
            continue;
        }

        Particle& particle = node.Value;
        particle.Life -= g_DeltaTime * 2.0f;

        particle.Transform.Position += (particle.Velocity + g_ConstantForce) * g_DeltaTime;
        particle.Transform.Size = vec2f(0.25f, 0.25f) * particle.Life + vec2f(0.05f, 0.05f);
        particle.Velocity *= 0.95f / (g_DeltaTime / 0.0166666f);

        const u8 alpha = particle.Life * 255;
        particle.ColorTint &= 0x00ffffff;
        particle.ColorTint |= (alpha << 24);
    }

    BrakeParticles.rescan();
}

void Player::render() const
{
    Renderer::get().render(AnimatedSpriteURI::Player,
                           m_currentFrame,
                           Camera::get().toNDCSpace(Transform),
                           FBL_COLOR_WHITE);
}

void Player::render_Trail() const
{
    if (m_playerTrailBuffer.size() == 0)
    {
        return;
    }

    bool flip = false;

    const vec2f uvSize = SpriteAtlas::at(SpriteURI::Plane).Size;
    const vec2f uvOffset = SpriteAtlas::at(SpriteURI::Plane).Offset;
    u32 colorTint = FBL_COLOR(0xcc, 0xcc, 0xcc, 0xff);
    float scale = 0.06f;

    Renderer::get().Position_VBO.push(Camera::get().toNDCSpace(vec2f(Transform.Position.x, Transform.Position.y + 0.5f)));

    Renderer::Color_UV_Data& color_uv = Renderer::get().Color_UV_VBO.push();
    color_uv.UV = uvSize * vec2f(1.0f * flip, 1.0f) + uvOffset;
    color_uv.ColorTint = colorTint;

    flip = !flip;
    const u8 alpha = ((colorTint >> 24) & 0xff) * 0.85f;
    colorTint &= 0x00ffffff;
    colorTint |= (alpha << 24);
    scale *= 1.15f;

    Renderer::get().m_currentSpriteCount += 1;

    vec2f lastPosition = *m_playerTrailBuffer.rbegin();
    for (auto positionIt = std::next(m_playerTrailBuffer.rbegin());
         positionIt != m_playerTrailBuffer.rend();
         ++positionIt)
    {
        const vec2f position = *positionIt;

        const vec2f trailSegment = position - lastPosition;
        const vec2f trailSegmentSideDir(glm::normalize(vec2f(trailSegment.y, -trailSegment.x)));

        Renderer::get().Position_VBO.push(Camera::get().toNDCSpace(trailSegmentSideDir * scale + lastPosition));
        Renderer::get().Position_VBO.push(Camera::get().toNDCSpace(-trailSegmentSideDir * scale + lastPosition));

        {
            Renderer::Color_UV_Data& color_uv = Renderer::get().Color_UV_VBO.push();
            color_uv.UV = uvSize * vec2f(1.0f * flip, 0.0f) + uvOffset;
            color_uv.ColorTint = colorTint;
        }

        {
            Renderer::Color_UV_Data& color_uv = Renderer::get().Color_UV_VBO.push();
            color_uv.UV = uvSize * vec2f(1.0f * flip, 1.0f) + uvOffset;
            color_uv.ColorTint = colorTint;
        }

        lastPosition = position;
        flip = !flip;

        const u8 alpha = ((colorTint >> 24) & 0xff) * 0.85f;
        colorTint &= 0x00ffffff;
        colorTint |= (alpha << 24);
        scale *= 1.15f;

        Renderer::get().m_currentSpriteCount += 2;
    }
}
