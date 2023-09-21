#include "Renderer/Renderer.hpp"
#include "spdlog/spdlog.h"
#include "webUi.hpp"

#include <allegro5/allegro_primitives.h>
namespace render
{

    Renderer::Renderer()
    {
    }

    Renderer::~Renderer()
    {
        if (m_render_thread.joinable())
        {
            spdlog::info("[Renderer] Joining render thread");
            m_render_thread.join();
        }
    }

    void Renderer::init()
    {

        al_init_primitives_addon();

        m_display = al_create_display(width, height);

        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP); // use memory bitmap for OSR buffer

        m_osr_buffer = al_create_bitmap(width, height);

        if (!m_display || !m_osr_buffer)
        {
            spdlog::error("Failed to create display or OSR bitmap buffer");
            exit(1);
        }

        // clear entire bitmap to white (osr buffer)
        auto locked_region = al_lock_bitmap(m_osr_buffer, ALLEGRO_PIXEL_FORMAT_RGBA_8888, ALLEGRO_LOCK_WRITEONLY);
        memset(locked_region->data, 0, width * height * locked_region->pixel_size);
        al_unlock_bitmap(m_osr_buffer);

        m_timer = al_create_timer(1.0 / fps);
        if (!m_timer)
        {
            spdlog::error("Failed to create timer");
            exit(1);
        }

        // Create the event queue
        m_event_queue = al_create_event_queue();
        if (!m_event_queue)
        {
            spdlog::error("Failed to create event queue");
            exit(1);
        }

        // Register event sources
        al_register_event_source(m_event_queue, al_get_display_event_source(m_display));
        al_register_event_source(m_event_queue, al_get_timer_event_source(m_timer));

        // Display a black screen, clear the screen once
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_flip_display();

        spdlog::info("Renderer initialized");
    }

    void Renderer::deinit()
    {

        al_destroy_timer(m_timer);
        m_timer = nullptr;
        al_destroy_event_queue(m_event_queue);
        m_event_queue = nullptr;
        al_destroy_bitmap(m_osr_buffer);
        m_osr_buffer = nullptr;
        al_destroy_display(m_display);
        m_display = nullptr;

        spdlog::info("Renderer deinitialized");
    }

    void Renderer::renderLoop()
    {

        // this is very important: OpenGL can only draw to a display if the display was created by that thread
        // This is becuase of the opengl context being tied to the thread. This is not a bug and stems from opengl
        // Opengl is not really multi-thread draw-safe
        this->init();

        // Start the timer
        al_start_timer(m_timer);
        m_running = true;

        // Game loop
        while (m_running)
        {
            ALLEGRO_EVENT event;

            // Fetch the event (if one exists)
            al_wait_for_event(m_event_queue, &event);

            // Handle the event

            switch (event.type)
            {
            case ALLEGRO_EVENT_TIMER:
                m_redraw_pending = true;
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                m_running = false;
                break;
            default:
                spdlog::warn("Renderer Unsupported event received: {}", event.type);
                break;
            }

            // Check if we need to redraw
            if (m_redraw_pending && al_is_event_queue_empty(m_event_queue))
            {
                // Clear the screen
                al_clear_to_color(al_map_rgba(0, 0, 0, 0));

                // Redraw

                static double delta_s = 0;
                // Get delta between redraws to know how much time passed between last redraw
                // Reason: Very simple way to decouple rendering FPS from game logic FPS/speed
                // Will cause problems with collision logic on low FPS / performance
                {
                    static auto last_delta_time_point = std::chrono::high_resolution_clock::now();
                    auto end = std::chrono::high_resolution_clock::now();
                    delta_s = std::chrono::duration<double, std::milli>(end - last_delta_time_point).count() / 1000; // why is chrono like this -.-
                    last_delta_time_point = end;
                }

                m_l_renderables.lock();
                for (auto &renderable : m_renderables)
                {
                    renderable->render(width,
                                       height, delta_s);
                }
                m_l_renderables.unlock();

                // draw OSR buffer over the screen,
                if (wui_rgba_bitmap != nullptr)
                {
                    // al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);

                    // copy over bitmap
                    auto locked_region = al_lock_bitmap(m_osr_buffer, ALLEGRO_PIXEL_FORMAT_RGBA_8888, ALLEGRO_LOCK_WRITEONLY);
                    memcpy(locked_region->data, wui_rgba_bitmap, width * height * locked_region->pixel_size);
                    al_unlock_bitmap(m_osr_buffer);

                    al_draw_bitmap(m_osr_buffer, 0, 0, 0);
                }
                else
                {
                    spdlog::warn("wui_rgba_bitmap is null");
                }

                al_flip_display();
                m_redraw_pending = false;
            }
        }

        this->deinit();
    }

    // CefBase interface

    ALLEGRO_DISPLAY *Renderer::getDisplay() const
    {
        return m_display;
    }

    void Renderer::shutdown()
    {
        spdlog::info("[Renderer] shutdown called");

        m_running = false;

        spdlog::info("[Renderer] shutdown complete");
    }

    void Renderer::start()
    {
        if (m_render_thread.joinable())
        {
            spdlog::warn("[Renderer] already running");
            return;
        }

        spdlog::info("[Renderer] starting");

        assert(wui::startWui(&wui_rgba_bitmap, width, height) && "Failed to start WUI");

        m_render_thread = std::thread(&Renderer::renderLoop, this);
    }

    void Renderer::waitUntilEnd()
    {
        spdlog::info("[Renderer] waiting until end");
        if (m_render_thread.joinable())
        {
            m_render_thread.join();
        }
        else
        {
            spdlog::warn("[Renderer] not running");
        }
        spdlog::info("[Renderer] wait complete");
    }

}