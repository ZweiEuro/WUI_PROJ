#include "webUiInput.hpp"
#include <allegro5/allegro.h>

namespace input
{

    // allegro doesn't fully adhere to posix or any code standard for receiving events.

    wui::wui_err_t handleKeyEvent(const wui::wui_tab_id_t &tab_id, ALLEGRO_EVENT &event);
    wui::wui_mouse_event_t convertMouseEvent(ALLEGRO_EVENT &event);
}
