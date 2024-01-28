

#include "Input/InputConversion.hpp"
#include "spdlog/spdlog.h"
namespace input
{
    wui::wui_mouse_event_t convertMouseEvent(ALLEGRO_EVENT &event)
    {

        return (wui::wui_mouse_event_t){
            .x = event.mouse.x,
            .y = event.mouse.y,
        };
    }

    uint32_t convertAllegroModifiersToWUI(uint32_t al_mods)
    {
        uint32_t ret = 0;

        if (al_mods & ALLEGRO_KEYMOD_SHIFT)
        {
            ret |= wui::EVENTFLAG_SHIFT_DOWN;
        }

        if (al_mods & ALLEGRO_KEYMOD_CTRL)
        {
            ret |= wui::EVENTFLAG_CONTROL_DOWN;
        }

        if (al_mods & ALLEGRO_KEYMOD_ALT)
        {
            ret |= wui::EVENTFLAG_ALT_DOWN;
        }

        if (al_mods & ALLEGRO_KEYMOD_ALTGR)
        {
            ret |= wui::EVENTFLAG_ALTGR_DOWN;
        }

        if (al_mods & ALLEGRO_KEYMOD_COMMAND)
        {
            ret |= wui::EVENTFLAG_COMMAND_DOWN;
        }

        if (al_mods & ALLEGRO_KEYMOD_CAPSLOCK)
        {
            ret |= wui::EVENTFLAG_CAPS_LOCK_ON;
        }

        if (al_mods & ALLEGRO_KEYMOD_NUMLOCK)
        {
            ret |= wui::EVENTFLAG_NUM_LOCK_ON;
        }

        return ret;
    }

    wui::wui_err_t handleKeyEvent(const wui::wui_tab_id_t &tab_id, ALLEGRO_EVENT &event)
    {
        // careful when using allegro!
        // allegro handles the different events in a mixed fashion

        auto eventString = [](ALLEGRO_EVENT_TYPE type)
        {
            switch (type)
            {
            case ALLEGRO_EVENT_KEY_DOWN:
                return "DOWN";
            case ALLEGRO_EVENT_KEY_UP:
                return "  UP";
            case ALLEGRO_EVENT_KEY_CHAR:
                return "CHAR";
            default:
                return "UNKNOWN";
            }
        };

        if (
            event.keyboard.type != ALLEGRO_EVENT_KEY_CHAR)
        {
            return wui::WUI_OK;
        }

        spdlog::info("[Input] {} convert Key event: type: {} unichar: {:8d} 0x{:2X} modifiers: 0x{:2X} keycode: {} ({})",
                     eventString(event.keyboard.type),
                     event.keyboard.type,
                     event.keyboard.unichar,
                     event.keyboard.unichar,
                     event.keyboard.modifiers,
                     event.keyboard.keycode,
                     al_keycode_to_name(event.keyboard.keycode));

        wui::wui_key_event_t ev = {
            .modifiers = convertAllegroModifiersToWUI(event.keyboard.modifiers),
            .repeat = event.keyboard.repeat,
        };

        if (event.keyboard.unichar == 0)
        {
            ev.windows_key_code = event.keyboard.keycode;

            switch (event.keyboard.keycode)
            {
            case ALLEGRO_KEY_LEFT:
                ev.windows_key_code = wui::VKEY_LEFT;
                break;

            case ALLEGRO_KEY_RIGHT:
                ev.windows_key_code = wui::VKEY_RIGHT;
                break;

            case ALLEGRO_KEY_UP:
                ev.windows_key_code = wui::VKEY_UP;
                break;

            case ALLEGRO_KEY_DOWN:
                ev.windows_key_code = wui::VKEY_DOWN;
                break;

            default:
                break;
            }
        }
        else
        {
            ev.native_key_code = event.keyboard.unichar;

            if (ev.modifiers & wui::EVENTFLAG_CONTROL_DOWN &&
                event.keyboard.keycode == event.keyboard.unichar)
            {
                // When control is pressed allegro sends its own keycode as native and unichar entry
                // this essentially means the native keycode is useless and needs to be adjusted by hand or discarded.
                // We opt to discard it when the two are not equal when control is pressed
                return wui::WUI_OK;
            }
            return wui::sendKeyEvent(tab_id, ev);
        }
    }
}