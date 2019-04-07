#include "pch.h"
#include "TundraApplication.h"

#include "SDL.h"
#include "SDL_image.h"
#include "Fabula/Graphics/Renderer.h"
#include "Fabula/Graphics/API/opengl.h"
#include "Fabula/Layer/GameLayer.h"
#include "Fabula/Library/Singleton.h"
#include "Fabula/Layer/LayerStack.h"
#include "Fabula/Layer/ApplicationLayer.h"
#include "Fabula/Layer/HUDLayer.h"
#include "Fabula/Layer/Event/EventBus.h"
#include "Fabula/Graphics/Text/TextRenderer.h"

#ifdef FBL_PLATFORM_WIN32
const u32 WinFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
#else
const u32 WinFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
#endif

namespace
{

void poll_events()
{
    EventBus& eventBus = EventBus::get();

    SDL_Event eventBuffer[4];
    SDL_PumpEvents();
    while (true)
    {
        const i32 eventsCount = SDL_PeepEvents(eventBuffer,
                                               ARRLEN(eventBuffer),
                                               SDL_GETEVENT,
                                               SDL_FIRSTEVENT,
                                               SDL_LASTEVENT);
        if (eventsCount == 0)
        {
            break;
        }

        if (eventsCount < 0)
        {
            REVEAL_SDL_ERROR("SDL Failed on peeping events")
        }

        FOR(eventsCount)
        {
            SDL_Event& event = eventBuffer[index];
            switch (event.type)
            {
            case SDL_QUIT:
            {
                fbl::g_Application->Terminate();
            } break;
            case SDL_MOUSEBUTTONDOWN:
#if 0
            case SDL_FINGERDOWN: // TODO: invastigate
#endif
            {
                ClickEvent* clickEvent = eventBus.enqueueEvent<ClickEvent>();
                if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                    i32 w;
                    i32 h;
                    SDL_GetWindowSize(fbl::g_Application->GetSDLWindow(), &w, &h);

                    clickEvent->NDCXPos = event.button.x / AS(float, w);
                    clickEvent->NDCYPos = event.button.y / AS(float, h);
                }
                else
                {
                    clickEvent->NDCXPos = event.tfinger.x;
                    clickEvent->NDCYPos = event.tfinger.y;
                }

                SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                             "Click (%.3f, %.3f)",
                             clickEvent->NDCXPos,
                             clickEvent->NDCYPos);
            } break;
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_SHOWN:
                case SDL_WINDOWEVENT_EXPOSED:
                case SDL_WINDOWEVENT_MAXIMIZED:
                case SDL_WINDOWEVENT_RESTORED:
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    WindowFocusEvent* focusEvent = eventBus.enqueueEvent<WindowFocusEvent>();
                    focusEvent->Focused = true;

                    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Window focus gained");
                } break;
                case SDL_WINDOWEVENT_HIDDEN:
                case SDL_WINDOWEVENT_MINIMIZED:
                case SDL_WINDOWEVENT_FOCUS_LOST:
                {
                    WindowFocusEvent* focusEvent = eventBus.enqueueEvent<WindowFocusEvent>();
                    focusEvent->Focused = false;

                    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Window focus lost");
                } break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    WindowResizedEvent* resizedEvent = eventBus.enqueueEvent<WindowResizedEvent>();
                    resizedEvent->Width = event.window.data1;
                    resizedEvent->Height = event.window.data2;

                    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                                 "Resized (%ix%i)",
                                 resizedEvent->Width,
                                 resizedEvent->Height);
                } break;
                case SDL_WINDOWEVENT_CLOSE:
                {
                    event.type = SDL_QUIT;
                    SDL_PushEvent(&event);
                } break;
                }
            } break;
            }
        }
    }
}

}

void TundraApplication::Init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        REVEAL_SDL_ERROR("SDL Initialization failed")
    }

    IMG_Init(IMG_INIT_PNG);

    const i32 winWidth = 450;
    const i32 winHeight = 800;

#ifdef FBL_PLATFORM_WIN32
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    m_SDLWindow = SDL_CreateWindow("Tundra",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   winWidth,
                                   winHeight,
                                   WinFlags);
    if (m_SDLWindow == nullptr)
    {
        REVEAL_SDL_ERROR("SDL window creation failed")
    }

    m_SDL_GLContext = SDL_GL_CreateContext(m_SDLWindow);

    fbl_init_opengl();

    SDL_GL_SetSwapInterval(1);
#ifdef _DEBUG
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
#endif

    LayerStack& layers = LayerStack::get();
    layers.push<ApplicationLayer>();
    g_Game = &layers.push<GameLayer>();
    layers.push<HUDLayer>();

    m_IsRunning = Renderer::get().init();
    m_IsRunning = m_IsRunning && TextRenderer::get().init();

    assert(m_IsRunning);
}

void TundraApplication::Shutdown()
{
    Renderer::get().shutdown();
    TextRenderer::get().shutdown();

    SDL_GL_DeleteContext(m_SDL_GLContext);
    SDL_DestroyWindow(m_SDLWindow);
    IMG_Quit();
    SDL_Quit();
}

void TundraApplication::Run()
{
    u64 lastPerfCounter = SDL_GetPerformanceCounter();
    const u64 frequency = SDL_GetPerformanceFrequency();
    i32 fps = 0;
    float elapsedTime = 0.0f;

    while (m_IsRunning)
    {
        poll_events();

        LayerStack& layers = LayerStack::get();

        layers.update();
        layers.render();

        EventBus::get().flushEvents();

        SDL_GL_SwapWindow(m_SDLWindow);

        ++fps;
        const u64 now = SDL_GetPerformanceCounter();
        g_DeltaTime = (now - lastPerfCounter) / (float)frequency;
        lastPerfCounter = now;

        elapsedTime += g_DeltaTime;
        if (elapsedTime > 1.0f)
        {
            elapsedTime -= 1.0f;
            //SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "FPS: %i, %fms", fps, 1000.f / fps);
            fps = 0;
        }
    }
}

std::unique_ptr<fbl::IApplication> fbl::create_application()
{
    return std::make_unique<TundraApplication>();
}
