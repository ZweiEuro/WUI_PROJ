#include "Input/Input.hpp"
#include "Enums.hpp"
#include "webUiInput.hpp"

#include <spdlog/spdlog.h>

namespace input
{
    // Game eveng queue (keyboard, mouse etc)
    ALLEGRO_EVENT_QUEUE *m_hardware_event_sources_queue;

    // Smaller listeners to more easily wait for events without blocking
    // where the manager tells "wait for keys" if something was pressed or not
    ALLEGRO_EVENT_SOURCE m_manager_event_source;

    // When received cleanly stop whatever you are doing
    ALLEGRO_EVENT_SOURCE m_abort_event_source;

    // current mouse position
    vec2i m_mouse_state;
    std::mutex l_mouse_state;

    // listen to all input events
    std::thread m_input_thread;

    std::atomic<wui_enums::SubSystemStates> m_state(wui_enums::SubSystemStates::STARTING);

    // prototype
    void input_loop();

    vec2i get_mouse_position()
    {
        l_mouse_state.lock();
        auto ret = m_mouse_state;
        l_mouse_state.unlock();
        return ret;
    }

    bool wait_for_key(int keycode)
    {
        if (m_state != wui_enums::SubSystemStates::RUNNING)
        {
            spdlog::error("[Input] wait_for_key called while not running");
            return false;
        }

        spdlog::info("[Input] waiting for key {}", keycode);
        auto queue = al_create_event_queue();
        al_register_event_source(queue, &m_manager_event_source);
        al_register_event_source(queue, &m_abort_event_source);
        bool ret = false;
        while (true)
        {
            ALLEGRO_EVENT event;
            al_wait_for_event(queue, &event);

            if (event.type == USER_BASE_EVENT)
            {
                if (event.user.data1 == (int)wui_enums::SubSystemStates::SHUTTING_DOWN)
                {
                    ret = false;
                    goto wait_for_key_end;
                }
            }

            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == keycode)
                {
                    ret = true;
                    goto wait_for_key_end;
                }
            }
        }

    wait_for_key_end:
        al_unregister_event_source(queue, &m_manager_event_source);
        al_destroy_event_queue(queue);
        return ret;
    }

    bool wait_for_mouse_button(int button, vec2i &mouse_pos)
    {

        if (m_state != wui_enums::SubSystemStates::RUNNING)
        {
            spdlog::error("[Input] wait_for_mouse_button called while not running");
            return false;
        }

        spdlog::info("[Input] waiting for mouse button {}", button);

        auto queue = al_create_event_queue();
        al_register_event_source(queue, &m_manager_event_source);
        al_register_event_source(queue, &m_abort_event_source);
        bool ret = false;
        while (true)
        {
            ALLEGRO_EVENT event;
            al_wait_for_event(queue, &event);

            if (event.type == USER_BASE_EVENT)
            {
                if (event.user.data1 == (int)wui_enums::SubSystemStates::SHUTTING_DOWN)
                {
                    ret = false;
                    goto wait_for_mouse_button_end;
                }
            }

            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
            {
                if (event.mouse.button & button)
                {
                    mouse_pos = {event.mouse.x, event.mouse.y};
                    ret = true;
                    goto wait_for_mouse_button_end;
                }
            }
        }

    wait_for_mouse_button_end:
        al_unregister_event_source(queue, &m_manager_event_source);
        al_destroy_event_queue(queue);

        return ret;
    }

    wui::wui_mouse_event_t convertMouseEvent(ALLEGRO_EVENT &event)
    {

        return (wui::wui_mouse_event_t){
            .x = event.mouse.x,
            .y = event.mouse.y,
            .modifiers = 0,
        };
    }

    void input_loop()
    {
        // start the main receiving loop
        m_state = wui_enums::SubSystemStates::RUNNING;

        while (m_state == wui_enums::SubSystemStates::RUNNING)
        {
            ALLEGRO_EVENT event;

            // Fetch the event (if one exists)
            al_wait_for_event(m_hardware_event_sources_queue, &event);

            // Handle the event

            switch (event.type)
            {
            case ALLEGRO_EVENT_MOUSE_AXES:

            {
                l_mouse_state.lock();
                m_mouse_state.x = event.mouse.x;
                m_mouse_state.y = event.mouse.y;
                l_mouse_state.unlock();

                const wui::wui_mouse_event_t ev = convertMouseEvent(event);

                wui::CEFSendMouseMoveEvent(ev, false);
            }

            break;
            case ALLEGRO_EVENT_KEY_DOWN:
                spdlog::info("[Input] key down {}", al_keycode_to_name(event.keyboard.keycode));

                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            {
                spdlog::info("[Input] mouse button down {}, @ {} {}", event.mouse.button == 1 ? "left" : "right", event.mouse.x, event.mouse.y);

                const wui::wui_mouse_event_t ev = convertMouseEvent(event);
                wui::CEFSendMouseClickEvent(ev, event.mouse.button == 1 ? wui::MBT_LEFT : wui::MBT_RIGHT, false, 1);

                break;
            }
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            {
                spdlog::info("[Input] mouse button up {}, @ {} {}", event.mouse.button == 1 ? "left" : "right", event.mouse.x, event.mouse.y);

                const wui::wui_mouse_event_t ev = convertMouseEvent(event);
                wui::CEFSendMouseClickEvent(ev, event.mouse.button == 1 ? wui::MBT_LEFT : wui::MBT_RIGHT, true, 1);
                break;
            }

            case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
            case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
            case ALLEGRO_EVENT_MOUSE_WARPED:
                // ignore since they have permission problems on ubuntu/kde
                break;

            case ALLEGRO_EVENT_KEY_CHAR:
            case ALLEGRO_EVENT_KEY_UP:
                // ignore all type events or key up events

                // TODO: Keyboard events
                break;

            default:
                // spdlog::info("[Input] event received: {}", event.type);
                break;
            }
            al_emit_user_event(&m_manager_event_source, &event, nullptr);
        }
        spdlog::info("[Input] Exit");

        // cleanup
        al_destroy_event_queue(m_hardware_event_sources_queue);
        al_destroy_user_event_source(&m_manager_event_source);
        al_destroy_user_event_source(&m_abort_event_source);
        al_uninstall_keyboard();
        al_uninstall_mouse();

        m_state = wui_enums::SubSystemStates::SHUTDOWN;
        return;
    }

    void start()
    {

        if (m_state != wui_enums::SubSystemStates::STARTING)
        {
            spdlog::error("[Input] start called while not in STARTING state");
            return;
        }

        assert(al_install_mouse() && "could not install mouse driver");
        assert(al_install_keyboard() && "could not install keyboard driver");

        assert(!m_input_thread.joinable() && "The input scanning thread should not be running");

        m_hardware_event_sources_queue = al_create_event_queue();

        al_init_user_event_source(&m_manager_event_source);
        al_init_user_event_source(&m_abort_event_source);

        al_register_event_source(m_hardware_event_sources_queue, al_get_mouse_event_source());
        al_register_event_source(m_hardware_event_sources_queue, al_get_keyboard_event_source());
        al_register_event_source(m_hardware_event_sources_queue, &m_abort_event_source); // make the main source also listen to abort events

        m_input_thread = std::thread([=]() -> void
                                     { input_loop(); });
    }

    void shutdown()
    {
        if (m_state != wui_enums::SubSystemStates::RUNNING)
        {
            spdlog::error("[Input] shutdown called while not running");
            return;
        }
        spdlog::info("[Input] shutdown called");

        m_state = wui_enums::SubSystemStates::SHUTTING_DOWN;

        ALLEGRO_EVENT ev = {};
        ev.user.data1 = (int)wui_enums::SubSystemStates::SHUTTING_DOWN;
        ev.type = USER_BASE_EVENT;

        al_emit_user_event(&m_abort_event_source, &ev, nullptr);

        m_input_thread.join();
    }

}