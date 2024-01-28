#pragma once
#include <allegro5/allegro.h>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include "Math/vec.hpp"

/**
 * @brief Input subsystem
 * @details Not a singleton sytem but a global one to not interfere with hardware installations
 *
 * In short: Starts a main thread of execution listening to all keyboard and mouse events. if another thread is created to wait for an event its triggered
 *
 *
 *
 */
namespace input
{
    // Start listening to all input and allow for waiting for keys
    void start();

    // Asyncronously shutdown all listeners, then the main one, and uninstall all allegro systems
    void shutdown();

    // Wait for external events
    // returns false if the system is shutting down
    bool wait_for_key(int keycode);

    // wait until _all_ codes are pressed at once
    bool wait_for_keys(std::vector<int> keycodes);

    // Wait for external events
    // returns false if the system is shutting down
    bool wait_for_mouse_button(int button, vec2i &mouse_pos);

    // get current mouse
    vec2i get_mouse_position();
}