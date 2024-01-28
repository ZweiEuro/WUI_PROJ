#include "Input/Input.hpp"
#include "Input/InputConversion.hpp"
#include "Enums.hpp"
#include "webUiInput.hpp"

#include <spdlog/spdlog.h>

#include <Renderer/Renderer.hpp>

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

    std::atomic<proj_enums::SubSystemStates> m_state(proj_enums::SubSystemStates::STARTING);

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

        return wait_for_keys({keycode});
    }

    bool wait_for_keys(std::vector<int> keycodes)
    {
        if (keycodes.size() == 0)
        {
            spdlog::error("[Input] wait_for_keys called with no keys");
            return false;
        }

        if (m_state != proj_enums::SubSystemStates::RUNNING)
        {
            spdlog::error("[Input] wait_for_key called while not running");
            return false;
        }

        std::vector<bool> pressed_keys = std::vector<bool>(keycodes.size(), false);

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
                if (event.user.data1 == (int)proj_enums::SubSystemStates::SHUTTING_DOWN)
                {
                    ret = false;
                    goto wait_for_keys_end;
                }
            }

            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                for (size_t i = 0; i < keycodes.size(); i++)
                {
                    if (event.keyboard.keycode == keycodes[i])
                    {
                        pressed_keys[i] = true;
                    }
                }
            }

            if (event.type == ALLEGRO_EVENT_KEY_UP)
            {
                for (size_t i = 0; i < keycodes.size(); i++)
                {
                    if (event.keyboard.keycode == keycodes[i])
                    {
                        pressed_keys[i] = false;
                    }
                }
            }

            if (std::all_of(pressed_keys.begin(), pressed_keys.end(), [](bool v)
                            { return v; }))
            {
                ret = true;
                goto wait_for_keys_end;
            }
        }

    wait_for_keys_end:
        al_unregister_event_source(queue, &m_manager_event_source);
        al_destroy_event_queue(queue);
        return ret;
    }

    bool wait_for_mouse_button(int button, vec2i &mouse_pos)
    {

        if (m_state != proj_enums::SubSystemStates::RUNNING)
        {
            spdlog::error("[Input] wait_for_mouse_button called while not running");
            return false;
        }

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
                if (event.user.data1 == (int)proj_enums::SubSystemStates::SHUTTING_DOWN)
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

    void input_loop()
    {
        // start the main receiving loop
        m_state = proj_enums::SubSystemStates::RUNNING;

        while (m_state == proj_enums::SubSystemStates::RUNNING)
        {
            ALLEGRO_EVENT event;

            // Fetch the event (if one exists)
            al_wait_for_event(m_hardware_event_sources_queue, &event);

            // Handle the event
            bool wasUiEvent = false;

            // When the "buttonDown" event fires over UI element, and it hit the UI
            // We also need to _always_ send the buttonUp event, even if it did not hit the UI -> using the force flag
            // This also allows up to detect "dragging"
            bool wuiButtonDown = false;
            bool wuiDragging = false;

            switch (event.type)
            {
            case ALLEGRO_EVENT_MOUSE_AXES:
            {
                l_mouse_state.lock();
                m_mouse_state.x = event.mouse.x;
                m_mouse_state.y = event.mouse.y;
                l_mouse_state.unlock();

                const wui::wui_mouse_event_t ev = convertMouseEvent(event);

                if (wuiButtonDown)
                {
                    wuiDragging = true;
                    // TODO: wui start dragging
                }

                wui::sendMouseMoveEvent(render::renderer.wui_tab_id, ev, false);
            }

            break;

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            {
                spdlog::info("[Input] mouse button down {}, @ {} {}", event.mouse.button == 1 ? "left" : "right", event.mouse.x, event.mouse.y);

                const wui::wui_mouse_event_t ev = convertMouseEvent(event);
                wasUiEvent = wui::sendMouseClickEvent(render::renderer.wui_tab_id, ev, event.mouse.button == 1 ? wui::MBT_LEFT : wui::MBT_RIGHT, false) == wui::WUI_HIT_UI;

                if (wasUiEvent)
                {
                    wuiButtonDown = true;
                }

                break;
            }
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            {
                spdlog::info("[Input] mouse button up {}, @ {} {}", event.mouse.button == 1 ? "left" : "right", event.mouse.x, event.mouse.y);

                const wui::wui_mouse_event_t ev = convertMouseEvent(event);

                wasUiEvent = wui::sendMouseClickEvent(render::renderer.wui_tab_id, ev, event.mouse.button == 1 ? wui::MBT_LEFT : wui::MBT_RIGHT, true, true) == wui::WUI_HIT_UI;

                if (wasUiEvent)
                {
                    wuiButtonDown = false;
                }

                if (wuiDragging)
                {
                    // WUI stop dragging
                    wuiDragging = false;
                }

                break;
            }

            case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
            case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
            case ALLEGRO_EVENT_MOUSE_WARPED:
                // ignore since they have permission problems on ubuntu/kde
                break;

            case ALLEGRO_EVENT_KEY_CHAR:
            case ALLEGRO_EVENT_KEY_UP:
            case ALLEGRO_EVENT_KEY_DOWN:
            {
                wui::wui_text_input_mode_t ret = wui::WUI_TEXT_INPUT_MODE_ERROR;
                WUI_ERROR_CHECK(wui::getCurrentTextInputMode(render::renderer.wui_tab_id, ret));

                if (ret != wui::WUI_TEXT_INPUT_MODE_NONE)
                {

                    if (event.keyboard.repeat)
                    {
                        break;
                    }

                    handleKeyEvent(render::renderer.wui_tab_id, event);
                    wasUiEvent = true;
                }
            }

            break;

            default:
                // spdlog::info("[Input] event received: {}", event.type);
                break;
            }

            if (!wasUiEvent)
            {

                al_emit_user_event(&m_manager_event_source, &event, nullptr);
            }
        }
        spdlog::info("[Input] Exit");

        // cleanup
        al_destroy_event_queue(m_hardware_event_sources_queue);
        al_destroy_user_event_source(&m_manager_event_source);
        al_destroy_user_event_source(&m_abort_event_source);
        al_uninstall_keyboard();
        al_uninstall_mouse();

        m_state = proj_enums::SubSystemStates::SHUTDOWN;
        return;
    }

    void start()
    {

        if (m_state != proj_enums::SubSystemStates::STARTING)
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
        if (m_state != proj_enums::SubSystemStates::RUNNING)
        {
            spdlog::error("[Input] shutdown called while not running");
            return;
        }
        spdlog::info("[Input] shutdown called");

        m_state = proj_enums::SubSystemStates::SHUTTING_DOWN;

        ALLEGRO_EVENT ev = {};
        ev.user.data1 = (int)proj_enums::SubSystemStates::SHUTTING_DOWN;
        ev.type = USER_BASE_EVENT;

        al_emit_user_event(&m_abort_event_source, &ev, nullptr);

        m_input_thread.join();
    }
}