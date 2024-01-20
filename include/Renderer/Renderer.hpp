#pragma once

#include <allegro5/allegro.h>
#include <memory>
#include <stdio.h>
#include <atomic>
#include <mutex>
#include <vector>
#include <thread>

#include "Objects/Renderable.hpp"

#include "webUiBinding.hpp"
#include "webUiTypes.hpp"

namespace render
{
    const size_t BASE_FPS = 60;
    const size_t BASE_WIDTH = 640;
    const size_t BASE_HEIGHT = 480;

    /**
     * A Renderer owns a display and a list of objects that are render-able
     * The display itself is the event source for closing the window, resizing, etc.
     *
     * Multiple renderer may be instanced and started at the same time.
     *
     * Any instance also has an OCR buffer for testing the WGUI system library
     *
     */
    class Renderer
    {

    private:
        void init();
        void deinit();

        size_t height = BASE_HEIGHT;
        size_t width = BASE_WIDTH;
        size_t fps = BASE_FPS;

    public:
        wui::wui_tab_id_t wui_tab_id = 0;
        Renderer();
        ~Renderer();

    private:
        // Required always, physical display of allegro
        ALLEGRO_DISPLAY *m_display = NULL;

        // Display event loop, closing window, etc
        ALLEGRO_EVENT_QUEUE *m_event_queue = NULL; // Display event loop

        // FPS rerender timer
        ALLEGRO_TIMER *m_timer = NULL;

        // Gated control for stopping and communication:
        // Asyncronous stop
        std::atomic<bool> m_running = ATOMIC_VAR_INIT(false);
        std::atomic<bool> m_redraw_pending = ATOMIC_VAR_INIT(false);

        // Rendering loop for the game engine
        std::thread m_render_thread;
        void renderLoop();

        // Pointer to the current display bitmap, void* to RGBA( width * height * 4)
        void *wui_rgba_bitmap = nullptr;

    private: // OSR buffer rendering
        // Main off screen rendering buffer where the CEF will render into
        ALLEGRO_BITMAP *m_osr_buffer = NULL;
        std::mutex m_l_osr_buffer_lock;

    private:
        // Register of all game objects that are to be rendered
        // Note: Consider moving this into a entity management system and reference that system here
        std::mutex m_l_renderables;
        std::vector<std::shared_ptr<objects::Renderable>> m_renderables;

    public:
        // FrameListener interface
        ALLEGRO_DISPLAY *getDisplay() const;

        void shutdown();
        void start();
        void waitUntilEnd();

        int handleDeleteObject(const cJSON *load, cJSON *retval, std::string &exc);

        void restartWui();

        // object management
    public:
        void addObject(std::shared_ptr<objects::Renderable> renderable)
        {
            m_l_renderables.lock();
            m_renderables.push_back(renderable);
            m_l_renderables.unlock();
        }
    };

    extern Renderer renderer;
}
