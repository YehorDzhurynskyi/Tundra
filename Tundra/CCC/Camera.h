#pragma once

#include "Fabula/Layer/Event/Event.h"
#include "Fabula/Layer/LayerNode.h"

#include "Fabula/Library/Singleton.h"

class Camera final : public LayerNode, public Singleton<Camera>
{
friend class Singleton<Camera>;
private:
    Camera();

public:
    static const float g_MinimumVisibleWorldHeight;

    vec2f getScreenSize() const;
    vec2f getVisibleWorldBounds() const;

    Transform toWorldSpace(const Transform& screenSpace) const;
    Transform toScreenSpace(const Transform& worldSpace) const;
    Transform toNDCSpace(const Transform& worldSpace) const;
    vec2f toNDCSpace(const vec2f& worldSpace) const;

    vec2f screenSpace_To_NDCSpace(const vec2f& screenSpace) const;

protected:
    void onConnect(Layer& layer) override;
    void onWindowSizeChanged(i32 width, i32 height);

public:
    vec2f Position;
    float Zoom = 1.0f;

private:
    EventListener m_windowResizedListener;

    vec2f m_screenSize;
    vec2f m_visibleBounds;
};

