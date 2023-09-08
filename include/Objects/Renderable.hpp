#pragma once
#include <cstddef>
namespace objects
{
    class Renderable
    {
    public:
        virtual void render(const size_t displayWidth, const size_t displayHeight, const double delta_t) = 0;
    };
}