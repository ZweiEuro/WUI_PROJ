#pragma once
#include <allegro5/allegro.h>
#include <mutex>
#include <thread>
#include <atomic>
#include "Math/vec.hpp"

namespace input
{
    void start();
    void shutdown();

    // Wait for external events
    bool wait_for_key(int keycode);
    bool wait_for_mouse_button(int button, vec2i &mouse_pos);

    // get current mouse
    vec2i get_mouse_position();
}