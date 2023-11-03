#pragma once
#include <cstddef>
namespace objects
{

    struct Position
    {
        float x;
        float y;
    };

    class Renderable
    {
    private:
        static int generator_id;

        int m_id;

    protected:
        float m_x;
        float m_y;

        Renderable();

    public:
        virtual void
        render(const size_t displayWidth, const size_t displayHeight, const double delta_t) = 0;
        Position getPosition();
        int getId() const;
    };

}