
#pragma once
#include "Renderable.hpp"
#include <allegro5/allegro.h>

namespace objects
{

    class Ball : public Renderable
    {
    private:
        float m_x;
        float m_y;
        float m_radius;
        float m_speed;
        float m_angle;
        ALLEGRO_COLOR m_color;

    public:
        Ball(int x, int y);
        void render(const size_t displayWidth, const size_t displayHeight, const double delta_t) override;
    };

}