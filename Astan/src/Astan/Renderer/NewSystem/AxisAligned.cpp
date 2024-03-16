#include "aspch.h"
#include "AxisAligned.h"
#include "RenderUtils.h"
namespace Astan
{
    AxisAlignedBox::AxisAlignedBox(const glm::vec3& center, const glm::vec3& half_extent) { Update(center, half_extent); }

    void AxisAlignedBox::Merge(const glm::vec3& new_point)
    {
        Utils::makeFloor(m_min_corner, new_point);
        Utils::makeCeil(m_max_corner,new_point);

        m_center = 0.5f * (m_min_corner + m_max_corner);
        m_half_extent = m_center - m_min_corner;
    }

    void AxisAlignedBox::Update(const glm::vec3& center, const glm::vec3& half_extent)
    {
        m_center = center;
        m_half_extent = half_extent;
        m_min_corner = center - half_extent;
        m_max_corner = center + half_extent;
    }

}