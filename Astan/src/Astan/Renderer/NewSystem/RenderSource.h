#pragma once
#include <Astan/Renderer/EditorCamera.h>
#include <Platform/Vulkan/VulkanRHIResource.h>
#include "GlobalRender.h"
#include "RenderConfig.h"
#include <Astan/Scene/Component.h>

namespace Astan
{
	struct IBLResource
	{
		RHIImage* _brdfLUT_texture_image;
		RHIImageView* _brdfLUT_texture_image_view;
		RHISampler* _brdfLUT_texture_sampler;
		VmaAllocation _brdfLUT_texture_image_allocation;

		RHIImage* _irradiance_texture_image;
		RHIImageView* _irradiance_texture_image_view;
		RHISampler* _irradiance_texture_sampler;
		VmaAllocation _irradiance_texture_image_allocation;

		RHIImage* _specular_texture_image;
		RHIImageView* _specular_texture_image_view;
		RHISampler* _specular_texture_sampler;
		VmaAllocation _specular_texture_image_allocation;
	};

	struct IBLResourceData
	{
		void* _brdfLUT_texture_image_pixels;
		uint32_t             _brdfLUT_texture_image_width;
		uint32_t             _brdfLUT_texture_image_height;
		RHIFormat   _brdfLUT_texture_image_format;
		std::array<void*, 6> _irradiance_texture_image_pixels;
		uint32_t             _irradiance_texture_image_width;
		uint32_t             _irradiance_texture_image_height;
		RHIFormat   _irradiance_texture_image_format;
		std::array<void*, 6> _specular_texture_image_pixels;
		uint32_t             _specular_texture_image_width;
		uint32_t             _specular_texture_image_height;
		RHIFormat   _specular_texture_image_format;
	};

	struct ColorGradingResource
	{
		RHIImage* _color_grading_LUT_texture_image;
		RHIImageView* _color_grading_LUT_texture_image_view;
		VmaAllocation _color_grading_LUT_texture_image_allocation;
	};

	struct ColorGradingResourceData
	{
		void* _color_grading_LUT_texture_image_pixels;
		uint32_t           _color_grading_LUT_texture_image_width;
		uint32_t           _color_grading_LUT_texture_image_height;
		RHIFormat _color_grading_LUT_texture_image_format;
	};

	struct StorageBuffer
	{
		// limits
		uint32_t _min_uniform_buffer_offset_alignment{ 256 };
		uint32_t _min_storage_buffer_offset_alignment{ 256 };
		uint32_t _max_storage_buffer_range{ 1 << 27 };
		uint32_t _non_coherent_atom_size{ 256 };

		RHIBuffer* _global_upload_ringbuffer;
		RHIDeviceMemory* _global_upload_ringbuffer_memory;
		void* _global_upload_ringbuffer_memory_pointer;
		std::vector<uint32_t> _global_upload_ringbuffers_begin;
		std::vector<uint32_t> _global_upload_ringbuffers_end;
		std::vector<uint32_t> _global_upload_ringbuffers_size;

		RHIBuffer* _global_null_descriptor_storage_buffer;
		RHIDeviceMemory* _global_null_descriptor_storage_buffer_memory;

		// axis
		RHIBuffer* _axis_inefficient_storage_buffer;
		RHIDeviceMemory* _axis_inefficient_storage_buffer_memory;
		void* _axis_inefficient_storage_buffer_memory_pointer;
	};

	struct GlobalRenderResource
	{
		IBLResource          _ibl_resource;
		ColorGradingResource _color_grading_resource;
		StorageBuffer        _storage_buffer;
	};

	struct LevelIBLResourceDesc
	{
		SkyBoxIrradianceMap m_skybox_irradiance_map;
		SkyBoxSpecularMap   m_skybox_specular_map;
		std::string         m_brdf_map;
	};

	struct LevelColorGradingResourceDesc
	{
		std::string m_color_grading_map;
	};

	struct LevelResourceDesc
	{
		LevelIBLResourceDesc          m_ibl_resource_desc;
		LevelColorGradingResourceDesc m_color_grading_resource_desc;
	};

	class RenderSource
	{
	public:
		RenderSource();
		~RenderSource();


		void UpdateVisibleObjects(Scene* scene, Ref<EditorCamera> camera);
		void UpdateVisibleObjectsDirectionalLight(Scene* scene,Ref<EditorCamera> camera);
		void UpdateVisibleObjectsPointLight(Scene* scene);
		void UpdateVisibleObjectsMainCamera(Scene* scene,Ref<EditorCamera> camera);
		void UpdateVisibleObjectsAxis();
		void UpdateVisibleObjectsParticle();
		void UploadGlobalRenderResource(Ref<VulkanRendererAPI> rhi, LevelResourceDesc level_resource_desc);
		void UpdatePerFrameBuffer(Scene* scene,Ref<EditorCamera> camera);

		VulkanMesh& GetEntityMesh(RenderEntityComponent entity);
		VulkanPBRMaterial& GetEntityMaterial(RenderEntityComponent entity);
		
		void CreateAndMapStorageBuffer(Ref<VulkanRendererAPI> rhi);
		void CreateIBLSamplers(Ref<VulkanRendererAPI> rhi);
		void CreateIBLTextures(Ref<VulkanRendererAPI> rhi, std::array<Ref<TextureData>, 6> irradiance_maps, std::array<Ref<TextureData>, 6> specular_maps);
		Ref<TextureData> LoadTextureHDR(std::string file, int desired_channels = 4);
		Ref<TextureData> LoadTexture(std::string file, bool is_srgb = false);

		void ResetRingBufferOffset(uint8_t current_frame_index);

	public:
		// global rendering resource, include IBL data, global storage buffer
		GlobalRenderResource m_GlobalRenderResource;

		// storage buffer objects
		MeshPerframeStorageBufferObject    m_MeshPerframeStorageBufferObject;
		MeshPointLightShadowPerframeStorageBufferObject m_MeshPointLightShadowPerframeStorageBufferObject;
		MeshDirectionalLightShadowPerframeStorageBufferObject m_MeshDirectionalLightShadowPerframeStorageBufferObject;
		AxisStorageBufferObject                        m_AxisStorageBufferObject;
		MeshInefficientPickPerframeStorageBufferObject m_MeshInefficientPickPerframeStorageBufferObject;
		ParticleBillboardPerframeStorageBufferObject   m_ParticlebillboardPerframeStorageBufferObject;
		ParticleCollisionPerframeStorageBufferObject   m_ParticleCollisionPerframeStorageBufferObject;
		std::vector<RenderMeshNode> m_DirectionalLightVisibleMeshNodes;

		// cached mesh and material
		std::map<size_t, VulkanMesh>        m_VulkanMeshes;
		std::map<size_t, VulkanPBRMaterial> m_VulkanPbrMaterials;

		// axis, for editor
		std::optional<RenderEntityComponent> m_RenderAxis;

		// visible objects(updated per frame)
		std::vector<RenderMeshNode> m_DirectionalLightVisibleMeshNodes;
		std::vector<RenderMeshNode> m_PointLightsVisibleMeshNodes;
		std::vector<RenderMeshNode> m_MainCameraVisibleMeshNodes;
		RenderAxisNode              m_AxisNode;
	};
}