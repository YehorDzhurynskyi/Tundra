#include "pch.h"
#include "ApplicationLayer.h"

#include <Fabula/Graphics/Renderer.h>
#include <Fabula/CCC/Camera.h>

ApplicationLayer::ApplicationLayer()
{
    Renderer& renderer = Renderer::get();
    Camera& camera = Camera::get();

    renderer.connect(*this);
    camera.connect(*this);
}

void ApplicationLayer::update()
{}

void ApplicationLayer::render() const
{}
