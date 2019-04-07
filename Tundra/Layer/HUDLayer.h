#pragma once

#include <Fabula/Layer/Layer.h>
#include <Fabula/Layer/LayerNode.h>

class HUDLayer : public Layer, public LayerNode
{
public:
    HUDLayer();

    void update() override;
    void render() const override;

protected:
    void onConnect(Layer& layer) override;

protected:
    EventListener m_clickEventListener;
};
