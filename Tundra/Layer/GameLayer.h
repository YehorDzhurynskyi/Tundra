#pragma once

#include "Player.h"
#include "CCC/Camera.h"

#include <Fabula/Assets/SpriteAtlas.h>

#include <Fabula/Layer/Layer.h>

#include <Fabula/Library/Pool.h>

class Obstacle
{
public:
    void update();
    void render() const;

    bool isAlive() const;

public:
    SpriteURI SpriteURI;
    Transform Transform;
    u32 ColorTint;

private:
    bool m_visible = false;
    float m_scale = 0.0f;
};

class GameLayer final : public fbl::Layer
{
    friend class HUDLayer;

public:
    GameLayer();

    static const float g_MapWidth;
    static const float g_ChunkGenerationOffset;

    void update() override;
    void render() const override;

    const Player& getPlayer() const;

protected:
    void generateNextChunk();

    template<typename T>
    void generateChunk(const T& chunk)
    {
        const float globalYPos =
            m_player.Transform.Position.y +
            Camera::g_MinimumVisibleWorldHeight +
            g_ChunkGenerationOffset;

        for (const auto& localPosition : chunk)
        {
            Obstacle& obstacle = *m_obstacles.push();
            obstacle.Transform.Position.x = GameLayer::g_MapWidth * 0.5f * localPosition.x;
            obstacle.Transform.Position.y = Camera::g_MinimumVisibleWorldHeight * 0.5f * localPosition.y + globalYPos;
            obstacle.SpriteURI = SpriteURI::Tree; // AS(SpriteURI, rand() % 2 ? SpriteURI::Tree : SpriteURI::Rock);
            obstacle.ColorTint = FBL_COLOR(0xff, 0xff, 0xff, 0xff);

            switch (obstacle.SpriteURI)
            {
            case SpriteURI::Rock:
            {
                obstacle.Transform.Size = vec2f(1.75f, 1.75f) + rand01() * vec2f(1.0f, 1.0f);
            } break;
            case SpriteURI::Tree:
            {
                u32 treeColors[] {
                    FBL_COLOR(0x5a, 0xaf, 0x24, 0xff),
                    FBL_COLOR(0x19, 0x87, 0x60, 0xff),
                    FBL_COLOR(0x6b, 0xa0, 0x00, 0xff),
                    FBL_COLOR(0x62, 0x17, 0x75, 0xff),
                    FBL_COLOR(0xef, 0x53, 0x7d, 0xff)
                };

                obstacle.Transform.Size = vec2f(2.0f, 2.0f) + rand01() * vec2f(0.5f, 0.5f);
                obstacle.ColorTint = treeColors[rand() % ARRLEN(treeColors)];
            } break;
            default:
            {
                assert(!"Unrecognized obstacle");
            } break;
            }
        }
    }

protected:
    Player m_player;
    SpriteAtlas m_atlas;
    fbl::Pool<Obstacle, 100> m_obstacles;

#ifdef _DEBUG
    fbl::Pool<Obstacle, 10> m_Debug;
#endif
};

extern GameLayer* g_Game;
