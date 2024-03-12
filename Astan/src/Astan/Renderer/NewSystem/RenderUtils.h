#pragma once

namespace Astan
{
    class Scene;
    class EditorCamera;

    static inline uint32_t roundUp(uint32_t value, uint32_t alignment)
    {
        uint32_t temp = value + alignment - static_cast<uint32_t>(1);
        return (temp - temp % alignment);
    }

    // TODO: support cluster lighting
    struct ClusterFrustum
    {
        // we don't consider the near and far plane currently
        glm::vec4 m_plane_right;
        glm::vec4 m_plane_left;
        glm::vec4 m_plane_top;
        glm::vec4 m_plane_bottom;
        glm::vec4 m_plane_near;
        glm::vec4 m_plane_far;
    };

    struct BoundingBox
    {
        glm::vec3 min_bound{ std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max() };
        glm::vec3 max_bound{ std::numeric_limits<float>::min(),
                           std::numeric_limits<float>::min(),
                           std::numeric_limits<float>::min() };

        BoundingBox() {}

        BoundingBox(const glm::vec3& minv, const glm::vec3& maxv)
        {
            min_bound = minv;
            max_bound = maxv;
        }

        void merge(const BoundingBox& rhs)
        {
            makeFloor(min_bound, rhs.min_bound);
            makeFloor(max_bound, rhs.max_bound);
        }

        void merge(const glm::vec3& point)
        {
            makeFloor(min_bound, point);
            makeFloor(max_bound, point);
        }

        void makeFloor(glm::vec3& base,const glm::vec3& cmp)
        {
            if (cmp.x < base.x)
                base.x = cmp.x;
            if (cmp.y < base.y)
                base.y = cmp.y;
            if (cmp.z < base.z)
                base.z = cmp.z;
        }

        void makeCeil(glm::vec3& base, const glm::vec3& cmp)
        {
            if (cmp.x > base.x)
                base.x = cmp.x;
            if (cmp.y > base.y)
                base.y = cmp.y;
            if (cmp.z > base.z)
                base.z = cmp.z;
        }


    };

    struct BoundingSphere
    {
        glm::vec3   m_center;
        float     m_radius;
    };

    struct FrustumPoints
    {
        glm::vec3 m_frustum_points;
    };

    ClusterFrustum CreateClusterFrustumFromMatrix(glm::mat4 mat,
        float     x_left,
        float     x_right,
        float     y_top,
        float     y_bottom,
        float     z_near,
        float     z_far);

    bool TiledFrustumIntersectBox(ClusterFrustum const& f, BoundingBox const& b);

    BoundingBox BoundingBoxTransform(BoundingBox const& b, glm::mat4 const& m);

    bool BoxIntersectsWithSphere(BoundingBox const& b, BoundingSphere const& s);

    glm::mat4 CalculateDirectionalLightCamera(Scene& scene, EditorCamera& camera);
} // namespace Piccolo
