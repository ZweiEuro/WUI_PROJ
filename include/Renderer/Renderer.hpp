#pragma once

#include <allegro5/allegro.h>
#include <memory>
#include <stdio.h>
#include <atomic>
#include <mutex>
#include <vector>
#include <thread>

#include "Objects/Renderable.hpp"

namespace render
{
    const size_t BASE_FPS = 60;
    const size_t BASE_WIDTH = 640;
    const size_t BASE_HEIGHT = 480;

    class Renderer
    {

    private: // Singleton
        static std::atomic<Renderer *> pinstance;
        static std::mutex m_l_instance;

        Renderer();

    public:
        static Renderer &instance();

    private:
        // Required always, physical display of allegro
        ALLEGRO_DISPLAY *m_display = NULL;

        // Display event loop, closing window, etc
        ALLEGRO_EVENT_QUEUE *m_event_queue = NULL; // Display event loop

        // FPS rerender timer
        ALLEGRO_TIMER *m_timer = NULL;

        // Gated control for stopping and communication:
        std::atomic<bool> m_running = ATOMIC_VAR_INIT(false);
        std::atomic<bool> m_redraw_pending = ATOMIC_VAR_INIT(false);

        // Rendering loop for the game engine
        std::thread m_render_thread;
        void renderLoop();

    private: // OSR buffer rendering
        // Main off screen rendering buffer where the CEF will render into
        ALLEGRO_BITMAP *m_osr_buffer = NULL;
        // Lock for the osr buffer
        std::mutex m_l_osr_buffer_lock;

    private:
        // Register of all game objects that are to be rendered
        // Note: Consider moving this into a entity management system and reference that system here
        std::mutex m_l_renderables;
        std::vector<std::shared_ptr<objects::Renderable>> m_renderables;

    public:
        ~Renderer();

        // FrameListener interface
        ALLEGRO_DISPLAY *getDisplay() const;

        void shutdown();
        void start();
        void waitUntilEnd();

        // object management
    public:
        void addObject(std::shared_ptr<objects::Renderable> renderable)
        {
            m_l_renderables.lock();
            m_renderables.push_back(renderable);
            m_l_renderables.unlock();
        }
    };

}
