#include "pch.h"
#include "Camera.h"

#include <Fabula/IApplication.h>
#include <Fabula/Graphics/API/opengl.h>
#include "Layer/GameLayer.h"

#include "SDL_video.h"
#include "Layer/Event/Event.h"

const float Camera::g_MinimumVisibleWorldHeight = 24.0f;

Camera::Camera()
{
    i32 w, h;
    SDL_GetWindowSize(fbl::g_Application->GetSDLWindow(), &w, &h);
    onWindowSizeChanged(w, h);

    m_windowResizedListener.on(WindowResizedEvent::TypeID(), [this](const IEvent& event)
    {
        assert(event.GetEventTypeID() == WindowResizedEvent::TypeID());
        const WindowResizedEvent& windowResizedEvent = AS(const WindowResizedEvent&, event);
        onWindowSizeChanged(windowResizedEvent.Width, windowResizedEvent.Height);

        return true;
    });
}

void Camera::onConnect(Layer& layer)
{
    m_windowResizedListener.bind(layer);
}

void Camera::onWindowSizeChanged(i32 width, i32 height)
{
    FBL_GL_CALL(glViewport(0, 0, width, height));

    m_screenSize = vec2f(width, height);

    const float aspectRatio = m_screenSize.x / m_screenSize.y;
    const float hBound = GameLayer::g_MapWidth / aspectRatio;
    m_visibleBounds = vec2f(GameLayer::g_MapWidth, hBound);
}

vec2f Camera::getScreenSize() const
{
    return m_screenSize;
}

vec2f Camera::getVisibleWorldBounds() const
{
    const float scale = std::max<float>(g_MinimumVisibleWorldHeight / m_visibleBounds.y, 1.0f / Zoom);
    return m_visibleBounds * scale;
}

Transform Camera::toWorldSpace(const Transform& screenSpace) const
{
    return {};
}

Transform Camera::toScreenSpace(const Transform& worldSpace) const
{
    Transform transform;

    const vec2f halfScreenSize = getScreenSize() / 2.0f;

    transform.Position = (2.0f * (worldSpace.Position - Position)) / getVisibleWorldBounds();
    transform.Position *= halfScreenSize;
    transform.Position += halfScreenSize;

    transform.Size = worldSpace.Size / getVisibleWorldBounds();
    transform.Size *= getScreenSize();

    return transform;
}

Transform Camera::toNDCSpace(const Transform& worldSpace) const
{
    Transform transform;

    transform.Position = toNDCSpace(worldSpace.Position);
    transform.Size = worldSpace.Size / getVisibleWorldBounds();

    return transform;
}

vec2f Camera::toNDCSpace(const vec2f& worldSpace) const
{
    vec2f NDCPosition = (2.0f * (worldSpace - Position)) / getVisibleWorldBounds();
    NDCPosition.y *= -1.0f;

    return NDCPosition;
}

vec2f Camera::screenSpace_To_NDCSpace(const vec2f& screenSpace) const
{
    return (2.0f * (screenSpace / getScreenSize())) - 1.0f;
}
