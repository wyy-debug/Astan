#pragma once
#include "Astan/Core/Timestep.h"
#include "Astan/Core/UUID.h"
#include "Astan/Renderer/EditorCamera.h"
#include "entt.hpp"
#include <Astan/Renderer/NewSystem/RenderConfig.h>
#include "Component.h"
#include <vma/VmaAllocation.h>
#include <Astan/Renderer/NewSystem/GlobalRender.h>
class b2World;


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


	class Entity;
	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		void UpdateVisibleObjects(Ref<EditorCamera> camera);
		void UpdateVisibleObjectsDirectionalLight(Ref<EditorCamera> camera);
		void UpdateVisibleObjectsPointLight();
		void UpdateVisibleObjectsMainCamera(Ref<EditorCamera> camera);
		void UpdateVisibleObjectsAxis();
		void UpdateVisibleObjectsParticle();
		
		void UploadGlobalRenderResource(std::shared_ptr<VulkanRendererAPI> rhi, LevelResourceDesc level_resource_desc);

		void CreateAndMapStorageBuffer(std::shared_ptr<VulkanRendererAPI> rhi);

		void CreateIBLSamplers(std::shared_ptr<VulkanRendererAPI> rhi);

		void CreateIBLTextures(std::shared_ptr<VulkanRendererAPI> rhi, std::array<std::shared_ptr<TextureData>, 6> irradiance_maps, std::array<std::shared_ptr<TextureData>, 6> specular_maps);

		Ref<TextureData> LoadTextureHDR(std::string file, int desired_channels = 4);

		std::shared_ptr<TextureData> LoadTexture(std::string file, bool is_srgb = false);


		VulkanMesh& GetEntityMesh(RenderEntityComponent entity);

		VulkanPBRMaterial& GetEntityMaterial(RenderEntityComponent entity);

		void UpdatePerFrameBuffer(Ref<EditorCamera> camera);

		void ResetRingBufferOffset(uint8_t current_frame_index);

		Entity CreateEntity(const std::	string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateSimulation(Timestep ts, EditorCamera& camera);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity DuplicateEntity(Entity entity);

		Entity FindEntityByName(std::string_view name);
		Entity GetEntityByUUID(UUID uuid);

		Entity GetPrimaryCameraEntity();

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }
		void SetPaused(bool paused) { m_IsPaused = paused; }

		void Step(int frames = 1);

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void RenderScene(EditorCamera& camera);
	private:
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		std::unordered_map<UUID, entt::entity> m_EntityMap;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	public:
		entt::registry m_Registry;

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