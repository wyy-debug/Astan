#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "Platform/Vulkan/VulkanRendererAPI.h"
#include <Astan/Renderer/RenderEntity.h>
#include <Astan/Renderer/EditorCamera.h>


namespace Astan
{
    class RHI;
    class Scene;
    class Camera;

    class RenderResourceBase
    {
    public:
        virtual ~RenderResourceBase() {}

        virtual void Clear() = 0;

        virtual void UploadGlobalRenderResource(Ref<RendererAPI> rhi, LevelResourceDesc level_resource_desc) = 0;

        virtual void UploadGameObjectRenderResource(Ref<RendererAPI> rhi,
            RenderEntity         render_entity,
            RenderMeshData       mesh_data,
            RenderMaterialData   material_data) = 0;

        virtual void UploadGameObjectRenderResource(Ref<RendererAPI> rhi,
            RenderEntity         render_entity,
            RenderMeshData       mesh_data) = 0;

        virtual void UploadGameObjectRenderResource(Ref<RendererAPI> rhi,
            RenderEntity         render_entity,
            RenderMaterialData   material_data) = 0;

        // TODO: data caching
        Ref<TextureData> LoadTextureHDR(std::string file, int desired_channels = 4);
        Ref<TextureData> LoadTexture(std::string file, bool is_srgb = false);
        RenderMeshData               LoadMeshData(const MeshSourceDesc& source, AxisAlignedBox& bounding_box);
        RenderMaterialData           LoadMaterialData(const MaterialSourceDesc& source);
        AxisAlignedBox               GetCachedBoudingBox(const MeshSourceDesc& source) const;

    private:
        StaticMeshData LoadStaticMesh(std::string mesh_file, AxisAlignedBox& bounding_box);

        std::unordered_map<MeshSourceDesc, AxisAlignedBox> m_bounding_box_cache_map;
    };
} // namespace Piccolo
