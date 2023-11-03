#include "Objects/Renderable.hpp"

namespace objects

{
    int Renderable::generator_id = 0;

    Renderable::Renderable()
    {
        m_x = 0;
        m_y = 0;
        m_id = generator_id++;
    }

    Position Renderable::getPosition()
    {
        return {
            m_x,
            m_y};
    }

    int Renderable::getId() const { return m_id; }

}