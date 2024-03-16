#pragma once

#include <string>
#include <vector>
namespace Astan
{
    class Vertex
    {
        public:
            float px;
            float py;
            float pz;
            float nx;
            float ny;
            float nz;
            float tx;
            float ty;
            float tz;
            float u;
            float v;
    };

    class SkeletonBinding
    {
        public:
            int   index0;
            int   index1;
            int   index2;
            int   index3;
            float weight0;
            float weight1;
            float weight2;
            float weight3;
    };
    class MeshData
    {
        public:
            std::vector<Vertex>          vertex_buffer;
            std::vector<int>             index_buffer;
            std::vector<SkeletonBinding> bind;
    };

} // namespace Piccolo