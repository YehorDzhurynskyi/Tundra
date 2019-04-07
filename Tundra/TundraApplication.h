#pragma once

#include <Fabula/IApplication.h>
#include <SDL_video.h>

class TundraApplication : public fbl::IApplication
{
public:
    void Init() override;
    void Shutdown() override;
    void Run() override;

protected:
    SDL_GLContext m_SDL_GLContext;
};
