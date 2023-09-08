#include "Objects/Ball.hpp"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <spdlog/spdlog.h>

#include <cmath>

namespace objects
{

    Ball::Ball(int x, int y)
    {
        m_x = x;
        m_y = y;
        m_radius = 10 + rand() % 100;
        m_speed = 200 + rand() % 200;
        m_angle = 20 + rand() % 20;
        m_color = al_map_rgb(rand() % 255, rand() % 255, rand() % 255);

        // spdlog::info("Ball created at ({}, {}) with radius {}, speed {} and angle {}", m_x, m_y, m_radius, m_speed, m_angle);
    }

    void Ball::render(const size_t displayWidth, const size_t displayHeight, const double delta_t)
    {
        // change position based on speed and angle
        m_x += m_speed * delta_t * cos(m_angle);
        m_y += m_speed * delta_t * sin(m_angle);

        // bounce off walls
        if (m_x < 0)
        {
            m_x = 0;
            m_angle = M_PI - m_angle;
        }
        else if (m_x > displayWidth)
        {
            m_x = displayWidth;
            m_angle = M_PI - m_angle;
        }

        if (m_y < 0)
        {
            m_y = 0;
            m_angle = -m_angle;
        }
        else if (m_y > displayHeight)
        {
            m_y = displayHeight;
            m_angle = -m_angle;
        }

        // draw
        al_draw_filled_circle(m_x, m_y, m_radius, m_color);
    }

}