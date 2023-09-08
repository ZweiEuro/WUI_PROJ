#include "Input/Input.hpp"

#include <spdlog/spdlog.h>

namespace input
{
    // Game eveng queue (keyboard, mouse etc)
    ALLEGRO_EVENT_QUEUE *m_hardware_event_sources_queue;

    // Smaller listeners to more easily wait for events without blocking
    // where the manager tells "wait for keys" if something was pressed or not
    ALLEGRO_EVENT_SOURCE m_manager_event_source;

    // current mouse position
    vec2i m_mouse_state;
    std::mutex l_mouse_state;

    // listen to all input events
    std::thread m_input_thread;
    std::atomic<bool> m_initialized(false);
    std::atomic<bool> m_running(true);

    // prototype
    void input_loop();

    void init()
    {
        if (m_initialized.exchange(true))
            return;

        assert(al_install_mouse() && "could not install mouse driver");
        assert(al_install_keyboard() && "could not install keyboard driver");

        assert(!m_input_thread.joinable());

        m_hardware_event_sources_queue = al_create_event_queue();

        al_init_user_event_source(&m_manager_event_source);

        al_register_event_source(m_hardware_event_sources_queue, al_get_mouse_event_source());
        al_register_event_source(m_hardware_event_sources_queue, al_get_keyboard_event_source());

        m_input_thread = std::thread([=]() -> void
                                     { input_loop(); });
    }

    vec2i get_mouse_position()
    {
        l_mouse_state.lock();
        auto ret = m_mouse_state;
        l_mouse_state.unlock();
        return ret;
    }

    bool wait_for_key(int keycode)
    {
        auto queue = al_create_event_queue();
        al_register_event_source(queue, &m_manager_event_source);
        bool exit = false;
        while (!exit)
        {
            ALLEGRO_EVENT event;
            al_wait_for_event(queue, &event);
            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                exit = (event.keyboard.keycode == keycode);
            }
        }
        al_unregister_event_source(queue, &m_manager_event_source);
        al_destroy_event_queue(queue);
        return true;
    }

    bool wait_for_mouse_button(int button, vec2i &mouse_pos)
    {
        // spdlog::info("[Input] waiting for mouse button {}", button);

        auto queue = al_create_event_queue();
        al_register_event_source(queue, &m_manager_event_source);
        bool exit = false;
        while (!exit)
        {
            ALLEGRO_EVENT event;
            al_wait_for_event(queue, &event);

            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
            {
                exit = (event.mouse.button & button);
                if (exit)
                {
                    mouse_pos = {event.mouse.x, event.mouse.y};
                }
            }
        }
        al_unregister_event_source(queue, &m_manager_event_source);
        al_destroy_event_queue(queue);

        return true;
    }

    void input_loop()
    {

        while (m_running)
        {
            ALLEGRO_EVENT event;

            // Fetch the event (if one exists)
            al_wait_for_event(m_hardware_event_sources_queue, &event);

            // Handle the event

            switch (event.type)
            {
            case ALLEGRO_EVENT_MOUSE_AXES:

                l_mouse_state.lock();
                m_mouse_state.x = event.mouse.x;
                m_mouse_state.y = event.mouse.y;
                l_mouse_state.unlock();

                break;
            case ALLEGRO_EVENT_KEY_DOWN:
                // spdlog::info("[Input] key down {}", al_keycode_to_name(event.keyboard.keycode));
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                // spdlog::info("[Input] mouse button down {}, @ {} {}", event.mouse.button == 1 ? "left" : "right",                             event.mouse.x, event.mouse.y);
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                // spdlog::info("[Input] mouse button up {}, @ {} {}", event.mouse.button == 1 ? "left" : "right",                             event.mouse.x, event.mouse.y);
                break;

            case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
            case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
            case ALLEGRO_EVENT_MOUSE_WARPED:
                // ignore since they have permission problems on ubuntu/kde
                break;

            case ALLEGRO_EVENT_KEY_CHAR:
            case ALLEGRO_EVENT_KEY_UP:
                // ignore all type events or key up events

                break;

            default:
                // // spdlog::info("[Input] event received: {}", event.type);
                break;
            }
            al_emit_user_event(&m_manager_event_source, &event, nullptr);
        }
        // spdlog::info("[Input] Exit");
        return;
    }

}