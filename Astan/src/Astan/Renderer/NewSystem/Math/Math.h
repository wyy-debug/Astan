#pragma once
#include <glm/glm.hpp>
namespace Astan
{
    namespace Utils
    {
        inline void makeFloor(glm::vec3& base, const glm::vec3& cmp)
        {
            if (cmp.x < base.x)
                base.x = cmp.x;
            if (cmp.y < base.y)
                base.y = cmp.y;
            if (cmp.z < base.z)
                base.z = cmp.z;
        }

        inline void makeCeil(glm::vec3& base, const glm::vec3& cmp)
        {
            if (cmp.x > base.x)
                base.x = cmp.x;
            if (cmp.y > base.y)
                base.y = cmp.y;
            if (cmp.z > base.z)
                base.z = cmp.z;
        }
    }
}