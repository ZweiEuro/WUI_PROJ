#pragma once
#include <allegro5/allegro.h>

// User events from allegro are programmers events.
// We use it to coordinate a controlled shutdown between the systems.
namespace wui_enums
{
#define USER_BASE_EVENT ALLEGRO_GET_EVENT_TYPE('w', 'g', 'u', 'i') // random init

    enum class SubSystemStates
    {
        STARTING = USER_BASE_EVENT + 1,
        RUNNING,
        SHUTTING_DOWN,
        SHUTDOWN,

    };
}