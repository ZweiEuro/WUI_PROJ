#include "Renderer/Renderer.hpp"
#include "spdlog/spdlog.h"

#include <allegro5/allegro_primitives.h>
namespace render
{

#define FULL_REDRAW 1

    Renderer::Renderer()
    {
    }

    void Renderer::init()
    {

        if (this->inited.exchange(true))
        {
            spdlog::warn("Renderer already initialized");
            return;
        }

        if (!al_is_system_installed())
        {

            spdlog::error("Allegro not installed on Renderer start");
            al_init();
            al_init_primitives_addon();
        }

        m_display = al_create_display(BASE_WIDTH, BASE_HEIGHT);

        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP); // use memory bitmap for OSR buffer

        m_osr_buffer = al_create_bitmap(BASE_WIDTH, BASE_HEIGHT);

        if (!m_display || !m_osr_buffer)
        {
            spdlog::error("Failed to create display or OSR bitmap buffer");
            exit(1);
        }

        // clear entire bitmap to white (osr buffer)
        auto locked_region = al_lock_bitmap(m_osr_buffer, ALLEGRO_PIXEL_FORMAT_RGBA_8888, ALLEGRO_LOCK_WRITEONLY);
        memset(locked_region->data, 0, BASE_WIDTH * BASE_HEIGHT * locked_region->pixel_size);
        al_unlock_bitmap(m_osr_buffer);

        m_timer = al_create_timer(1.0 / BASE_FPS);
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

    Renderer::~Renderer()
    {
        al_destroy_display(m_display);
        al_destroy_event_queue(m_event_queue);
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
                    renderable->render(al_get_display_width(m_display),
                                       al_get_display_height(m_display), delta_s);
                }
                m_l_renderables.unlock();

                // draw OSR buffer over the screen,
                //
                if (m_l_osr_buffer_lock.try_lock())
                {
                    // al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
                    al_draw_bitmap(m_osr_buffer, 0, 0, 0);
                    m_l_osr_buffer_lock.unlock();
                }
                else
                {
                    spdlog::warn("OSR buffer locked, skipping redraw");
                }

                al_flip_display();
                m_redraw_pending = false;
            }
        }

        // teardown
        al_destroy_timer(m_timer);
        al_destroy_display(m_display);
        al_destroy_bitmap(m_osr_buffer);
        al_destroy_event_queue(m_event_queue);
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
        m_render_thread.join();
        spdlog::info("[Renderer] shutdown complete");
    }

    void Renderer::start()
    {
        if (m_running)
        {
            spdlog::warn("[Renderer] already running");
            return;
        }

        spdlog::info("[Renderer] starting");
        m_running = true;
        m_render_thread = std::thread(&Renderer::renderLoop, this);
    }

    void Renderer::waitUntilEnd()
    {
        spdlog::info("[Renderer] waiting until end");
        m_render_thread.join();
        spdlog::info("[Renderer] wait complete");
    }

}