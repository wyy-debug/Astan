#include "aspch.h"
#include "RenderSource.h"
#include "RenderUtils.h"
#include "Astan/Scene/Scene.h"
#include "Astan/Scene/Entity.h"

namespace Astan
{
	RenderSource::RenderSource()
	{
	}
	RenderSource::~RenderSource()
	{
	}

	void RenderSource::UpdateVisibleObjects(Scene* scene,Ref<EditorCamera> camera)
	{
		UpdateVisibleObjectsDirectionalLight(scene,camera);
		UpdateVisibleObjectsPointLight(scene);
		UpdateVisibleObjectsMainCamera(scene,camera);
		UpdateVisibleObjectsAxis();
		UpdateVisibleObjectsParticle();
	}

	void RenderSource::UpdateVisibleObjectsDirectionalLight(Scene* scene,Ref<EditorCamera> camera)
	{

		glm::mat4 directionalLightProjView = CalculateDirectionalLightCamera(scene, *camera);

		m_MeshPerframeStorageBufferObject.directional_light_proj_view = directionalLightProjView;
		m_MeshDirectionalLightShadowPerframeStorageBufferObject.light_proj_view = directionalLightProjView;

		m_DirectionalLightVisibleMeshNodes.clear();

		ClusterFrustum frustum =
			CreateClusterFrustumFromMatrix(directionalLightProjView, -1.0, 1.0, -1.0, 1.0, 0.0, 1.0);

		auto view = scene->GetAllEntitiesWith<RenderEntityComponent, IDComponent>();
		for (auto e : view)
		{
			Entity entity = { e, scene };
			
			auto& RenderEntity = entity.GetComponent<RenderEntityComponent>();
			BoundingBox mesh_asset_bounding_box{ RenderEntity.m_BoundingBox.getMinCorner(),
												 RenderEntity.m_BoundingBox.getMaxCorner() };

			if (TiledFrustumIntersectBox(frustum, BoundingBoxTransform(mesh_asset_bounding_box, RenderEntity.m_ModelMatrix)))
			{
				m_DirectionalLightVisibleMeshNodes.emplace_back();
				RenderMeshNode& temp_node = m_DirectionalLightVisibleMeshNodes.back();

				temp_node.model_matrix = &RenderEntity.m_ModelMatrix;

				AS_CORE_ASSERT(RenderEntity.m_JointMatrices.size() <= s_mesh_vertex_blending_max_joint_count);
				if (!RenderEntity.m_JointMatrices.empty())
				{
					temp_node.joint_count = static_cast<uint32_t>(RenderEntity.m_JointMatrices.size());
					temp_node.joint_matrices = RenderEntity.m_JointMatrices.data();
				}

				temp_node.node_id = RenderEntity.m_InstanceId;

				VulkanMesh& mesh_asset = GetEntityMesh(RenderEntity);
				temp_node.ref_mesh = &mesh_asset;
				temp_node.enable_vertex_blending = RenderEntity.m_EnableVertexBlending;

				VulkanPBRMaterial& material_asset = GetEntityMaterial(RenderEntity);
				temp_node.ref_material = &material_asset;
			}
		}

	}


	void RenderSource::UpdateVisibleObjectsPointLight(Scene* scene)
	{
		m_PointLightsVisibleMeshNodes.clear();

		std::vector<BoundingSphere> point_lights_bounding_spheres;

		auto view = scene->GetAllEntitiesWith<PointLightComponent>();
		uint32_t point_light_num = view.size();
		point_lights_bounding_spheres.resize(point_light_num);
		{
			int i = 0;
			for (auto e : view)
			{
				Entity entity = { e, scene };

				auto& pointLight = entity.GetComponent<PointLightComponent>();
				point_lights_bounding_spheres[i].m_center = pointLight.Position;
				point_lights_bounding_spheres[i].m_radius = pointLight.calculteRadius();
				i++;
			}
		}

		auto view = scene->GetAllEntitiesWith<RenderEntityComponent>();
		for (auto e : view)
		{
			Entity entity = { e,scene };

			auto renderEntity = entity.GetComponent<RenderEntityComponent>();
			BoundingBox mesh_asset_bounding_box{ renderEntity.m_BoundingBox.getMinCorner(),
												 renderEntity.m_BoundingBox.getMaxCorner() };

			bool intersect_with_point_lights = true;
			for (size_t i = 0; i < point_light_num; i++)
			{
				if (!BoxIntersectsWithSphere(BoundingBoxTransform(mesh_asset_bounding_box, renderEntity.m_ModelMatrix),
					point_lights_bounding_spheres[i]))
				{
					intersect_with_point_lights = false;
					break;
				}
			}

			if (intersect_with_point_lights)
			{
				m_PointLightsVisibleMeshNodes.emplace_back();
				RenderMeshNode& temp_node = m_PointLightsVisibleMeshNodes.back();

				temp_node.model_matrix = &renderEntity.m_ModelMatrix;

				AS_CORE_ASSERT(renderEntity.m_JointMatrices.size() <= s_mesh_vertex_blending_max_joint_count);
				if (!renderEntity.m_JointMatrices.empty())
				{
					temp_node.joint_count = static_cast<uint32_t>(renderEntity.m_JointMatrices.size());
					temp_node.joint_matrices = renderEntity.m_JointMatrices.data();
				}
				temp_node.node_id = renderEntity.m_InstanceId;

				VulkanMesh& mesh_asset = GetEntityMesh(renderEntity);
				temp_node.ref_mesh = &mesh_asset;
				temp_node.enable_vertex_blending = renderEntity.m_EnableVertexBlending;

				VulkanPBRMaterial& material_asset = GetEntityMaterial(renderEntity);
				temp_node.ref_material = &material_asset;
			}
		}

	}

	void RenderSource::UpdateVisibleObjectsMainCamera(Scene* scene, Ref<EditorCamera> camera)
	{
		m_MainCameraVisibleMeshNodes.clear();

		glm::mat4 view_matrix = camera->GetViewMatrix();
		glm::mat4 proj_matrix = camera->GetProjection();
		glm::mat4 proj_view_matrix = proj_matrix * view_matrix;

		ClusterFrustum f = CreateClusterFrustumFromMatrix(proj_view_matrix, -1.0, 1.0, -1.0, 1.0, 0.0, 1.0);

		auto view = scene->GetAllEntitiesWith<RenderEntityComponent>();
		for (auto e : view)
		{
			Entity entity = { e,scene };
			auto renderEntity = entity.GetComponent<RenderEntityComponent>();

			BoundingBox mesh_asset_bounding_box{ renderEntity.m_BoundingBox.getMinCorner(),
												 renderEntity.m_BoundingBox.getMaxCorner() };

			if (TiledFrustumIntersectBox(f, BoundingBoxTransform(mesh_asset_bounding_box, renderEntity.m_ModelMatrix)))
			{
				m_MainCameraVisibleMeshNodes.emplace_back();
				RenderMeshNode& temp_node = m_MainCameraVisibleMeshNodes.back();
				temp_node.model_matrix = &renderEntity.m_ModelMatrix;

				assert(renderEntity.m_JointMatrices.size() <= s_mesh_vertex_blending_max_joint_count);
				if (!renderEntity.m_JointMatrices.empty())
				{
					temp_node.joint_count = static_cast<uint32_t>(renderEntity.m_JointMatrices.size());
					temp_node.joint_matrices = renderEntity.m_JointMatrices.data();
				}
				temp_node.node_id = renderEntity.m_InstanceId;

				VulkanMesh& mesh_asset = GetEntityMesh(renderEntity);
				temp_node.ref_mesh = &mesh_asset;
				temp_node.enable_vertex_blending = renderEntity.m_EnableVertexBlending;

				VulkanPBRMaterial& material_asset = GetEntityMaterial(renderEntity);
				temp_node.ref_material = &material_asset;
			}
		}

	}

	void RenderSource::UpdateVisibleObjectsAxis()
	{
		if (m_RenderAxis.has_value())
		{
			RenderEntityComponent& axis = *m_RenderAxis;

			m_AxisNode.model_matrix = axis.m_ModelMatrix;
			m_AxisNode.node_id = axis.m_InstanceId;

			VulkanMesh& mesh_asset = GetEntityMesh(axis);
			m_AxisNode.ref_mesh = &mesh_asset;
			m_AxisNode.enable_vertex_blending = axis.m_EnableVertexBlending;
		}
	}

	void RenderSource::UpdateVisibleObjectsParticle()
	{
		// TODO
	}


	VulkanMesh& RenderSource::GetEntityMesh(RenderEntityComponent entity)
	{
		size_t assetid = entity.m_MeshAssetId;

		auto it = m_VulkanMeshes.find(assetid);
		if (it != m_VulkanMeshes.end())
		{
			return it->second;
		}
		else
		{
			throw std::runtime_error("failed to get entity mesh");
		}
	}

	VulkanPBRMaterial& RenderSource::GetEntityMaterial(RenderEntityComponent entity)
	{
		size_t assetid = entity.m_MaterialAssetId;

		auto it = m_VulkanPbrMaterials.find(assetid);
		if (it != m_VulkanPbrMaterials.end())
		{
			return it->second;
		}
		else
		{
			throw std::runtime_error("failed to get entity material");
		}
	}

	void RenderSource::UploadGlobalRenderResource(Ref<VulkanRendererAPI> rhi, LevelResourceDesc level_resource_desc)
	{
		// create and map global storage buffer
		CreateAndMapStorageBuffer(rhi);

		// sky box irradiance
		SkyBoxIrradianceMap skybox_irradiance_map = level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map;
		Ref<TextureData> irradiace_pos_x_map = LoadTextureHDR(skybox_irradiance_map.m_positive_x_map);
		Ref<TextureData> irradiace_neg_x_map = LoadTextureHDR(skybox_irradiance_map.m_negative_x_map);
		Ref<TextureData> irradiace_pos_y_map = LoadTextureHDR(skybox_irradiance_map.m_positive_y_map);
		Ref<TextureData> irradiace_neg_y_map = LoadTextureHDR(skybox_irradiance_map.m_negative_y_map);
		Ref<TextureData> irradiace_pos_z_map = LoadTextureHDR(skybox_irradiance_map.m_positive_z_map);
		Ref<TextureData> irradiace_neg_z_map = LoadTextureHDR(skybox_irradiance_map.m_negative_z_map);

		// sky box specular
		SkyBoxSpecularMap            skybox_specular_map = level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map;
		Ref<TextureData> specular_pos_x_map = LoadTextureHDR(skybox_specular_map.m_positive_x_map);
		Ref<TextureData> specular_neg_x_map = LoadTextureHDR(skybox_specular_map.m_negative_x_map);
		Ref<TextureData> specular_pos_y_map = LoadTextureHDR(skybox_specular_map.m_positive_y_map);
		Ref<TextureData> specular_neg_y_map = LoadTextureHDR(skybox_specular_map.m_negative_y_map);
		Ref<TextureData> specular_pos_z_map = LoadTextureHDR(skybox_specular_map.m_positive_z_map);
		Ref<TextureData> specular_neg_z_map = LoadTextureHDR(skybox_specular_map.m_negative_z_map);

		// brdf
		Ref<TextureData> brdf_map = LoadTextureHDR(level_resource_desc.m_ibl_resource_desc.m_brdf_map);

		// create IBL samplers
		CreateIBLSamplers(rhi);

		// create IBL textures, take care of the texture order
		std::array<Ref<TextureData>, 6> irradiance_maps = { irradiace_pos_x_map,
																	   irradiace_neg_x_map,
																	   irradiace_pos_z_map,
																	   irradiace_neg_z_map,
																	   irradiace_pos_y_map,
																	   irradiace_neg_y_map };
		std::array<Ref<TextureData>, 6> specular_maps = { specular_pos_x_map,
																	 specular_neg_x_map,
																	 specular_pos_z_map,
																	 specular_neg_z_map,
																	 specular_pos_y_map,
																	 specular_neg_y_map };
		CreateIBLTextures(rhi, irradiance_maps, specular_maps);

		// create brdf lut texture
		rhi->CreateGlobalImage(
			m_GlobalRenderResource._ibl_resource._brdfLUT_texture_image,
			m_GlobalRenderResource._ibl_resource._brdfLUT_texture_image_view,
			m_GlobalRenderResource._ibl_resource._brdfLUT_texture_image_allocation,
			brdf_map->m_width,
			brdf_map->m_height,
			brdf_map->m_pixels,
			brdf_map->m_format);

		// color grading
		Ref<TextureData> color_grading_map =
			LoadTexture(level_resource_desc.m_color_grading_resource_desc.m_color_grading_map);

		// create color grading texture
		rhi->CreateGlobalImage(
			m_GlobalRenderResource._color_grading_resource._color_grading_LUT_texture_image,
			m_GlobalRenderResource._color_grading_resource._color_grading_LUT_texture_image_view,
			m_GlobalRenderResource._color_grading_resource._color_grading_LUT_texture_image_allocation,
			color_grading_map->m_width,
			color_grading_map->m_height,
			color_grading_map->m_pixels,
			color_grading_map->m_format);
	}

	void RenderSource::CreateAndMapStorageBuffer(Ref<VulkanRendererAPI> rhi)
	{
		VulkanRendererAPI* raw_rhi = static_cast<VulkanRendererAPI*>(rhi.get());
		StorageBuffer& _storage_buffer = m_GlobalRenderResource._storage_buffer;
		uint32_t       frames_in_flight = raw_rhi->k_max_frames_in_flight;

		RHIPhysicalDeviceProperties properties;
		rhi->GetPhysicalDeviceProperties(&properties);

		_storage_buffer._min_uniform_buffer_offset_alignment =
			static_cast<uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
		_storage_buffer._min_storage_buffer_offset_alignment =
			static_cast<uint32_t>(properties.limits.minStorageBufferOffsetAlignment);
		_storage_buffer._max_storage_buffer_range = properties.limits.maxStorageBufferRange;
		_storage_buffer._non_coherent_atom_size = properties.limits.nonCoherentAtomSize;

		// In Vulkan, the storage buffer should be pre-allocated.
		// The size is 128MB in NVIDIA D3D11
		// driver(https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0).
		uint32_t global_storage_buffer_size = 1024 * 1024 * 128;
		rhi->CreateBuffer(global_storage_buffer_size,
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			_storage_buffer._global_upload_ringbuffer,
			_storage_buffer._global_upload_ringbuffer_memory);

		_storage_buffer._global_upload_ringbuffers_begin.resize(frames_in_flight);
		_storage_buffer._global_upload_ringbuffers_end.resize(frames_in_flight);
		_storage_buffer._global_upload_ringbuffers_size.resize(frames_in_flight);
		for (uint32_t i = 0; i < frames_in_flight; ++i)
		{
			_storage_buffer._global_upload_ringbuffers_begin[i] = (global_storage_buffer_size * i) / frames_in_flight;
			_storage_buffer._global_upload_ringbuffers_size[i] =
				(global_storage_buffer_size * (i + 1)) / frames_in_flight -
				(global_storage_buffer_size * i) / frames_in_flight;
		}

		// axis
		rhi->CreateBuffer(sizeof(AxisStorageBufferObject),
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			_storage_buffer._axis_inefficient_storage_buffer,
			_storage_buffer._axis_inefficient_storage_buffer_memory);

		// null descriptor
		rhi->CreateBuffer(64,
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			0,
			_storage_buffer._global_null_descriptor_storage_buffer,
			_storage_buffer._global_null_descriptor_storage_buffer_memory);

		// TODO: Unmap when program terminates
		rhi->MapMemory(_storage_buffer._global_upload_ringbuffer_memory,
			0,
			RHI_WHOLE_SIZE,
			0,
			&_storage_buffer._global_upload_ringbuffer_memory_pointer);

		rhi->MapMemory(_storage_buffer._axis_inefficient_storage_buffer_memory,
			0,
			RHI_WHOLE_SIZE,
			0,
			&_storage_buffer._axis_inefficient_storage_buffer_memory_pointer);

		static_assert(64 >= sizeof(MeshVertex::VulkanMeshVertexJointBinding), "");
	}

	void RenderSource::CreateIBLSamplers(Ref<VulkanRendererAPI> rhi)
	{
		VulkanRendererAPI* raw_rhi = static_cast<VulkanRendererAPI*>(rhi.get());

		RHIPhysicalDeviceProperties physical_device_properties{};
		rhi->GetPhysicalDeviceProperties(&physical_device_properties);

		RHISamplerCreateInfo samplerInfo{};
		samplerInfo.sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = RHI_FILTER_LINEAR;
		samplerInfo.minFilter = RHI_FILTER_LINEAR;
		samplerInfo.addressModeU = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = RHI_TRUE;                                                // close:false
		samplerInfo.maxAnisotropy = physical_device_properties.limits.maxSamplerAnisotropy; // close :1.0f
		samplerInfo.borderColor = RHI_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = RHI_FALSE;
		samplerInfo.compareEnable = RHI_FALSE;
		samplerInfo.compareOp = RHI_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.maxLod = 0.0f;

		if (m_GlobalRenderResource._ibl_resource._brdfLUT_texture_sampler != RHI_NULL_HANDLE)
		{
			rhi->DestroySampler(m_GlobalRenderResource._ibl_resource._brdfLUT_texture_sampler);
		}

		if (rhi->CreateSampler(&samplerInfo, m_GlobalRenderResource._ibl_resource._brdfLUT_texture_sampler) != RHI_SUCCESS)
		{
			throw std::runtime_error("vk create sampler");
		}

		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 8.0f; // TODO: irradiance_texture_miplevels
		samplerInfo.mipLodBias = 0.0f;

		if (m_GlobalRenderResource._ibl_resource._irradiance_texture_sampler != RHI_NULL_HANDLE)
		{
			rhi->DestroySampler(m_GlobalRenderResource._ibl_resource._irradiance_texture_sampler);
		}

		if (rhi->CreateSampler(&samplerInfo, m_GlobalRenderResource._ibl_resource._irradiance_texture_sampler) != RHI_SUCCESS)
		{
			throw std::runtime_error("vk create sampler");
		}

		if (m_GlobalRenderResource._ibl_resource._specular_texture_sampler != RHI_NULL_HANDLE)
		{
			rhi->DestroySampler(m_GlobalRenderResource._ibl_resource._specular_texture_sampler);
		}

		if (rhi->CreateSampler(&samplerInfo, m_GlobalRenderResource._ibl_resource._specular_texture_sampler) != RHI_SUCCESS)
		{
			throw std::runtime_error("vk create sampler");
		}
	}

	void RenderSource::CreateIBLTextures(Ref<VulkanRendererAPI>                        rhi,
		std::array<Ref<TextureData>, 6> irradiance_maps,
		std::array<Ref<TextureData>, 6> specular_maps)
	{
		// assume all textures have same width, height and format
		uint32_t irradiance_cubemap_miplevels =
			static_cast<uint32_t>(
				std::floor(log2(std::max(irradiance_maps[0]->m_width, irradiance_maps[0]->m_height)))) +
			1;
		rhi->CreateCubeMap(
			m_GlobalRenderResource._ibl_resource._irradiance_texture_image,
			m_GlobalRenderResource._ibl_resource._irradiance_texture_image_view,
			m_GlobalRenderResource._ibl_resource._irradiance_texture_image_allocation,
			irradiance_maps[0]->m_width,
			irradiance_maps[0]->m_height,
			{ irradiance_maps[0]->m_pixels,
			 irradiance_maps[1]->m_pixels,
			 irradiance_maps[2]->m_pixels,
			 irradiance_maps[3]->m_pixels,
			 irradiance_maps[4]->m_pixels,
			 irradiance_maps[5]->m_pixels },
			irradiance_maps[0]->m_format,
			irradiance_cubemap_miplevels);

		uint32_t specular_cubemap_miplevels =
			static_cast<uint32_t>(
				std::floor(log2(std::max(specular_maps[0]->m_width, specular_maps[0]->m_height)))) +
			1;
		rhi->CreateCubeMap(
			m_GlobalRenderResource._ibl_resource._specular_texture_image,
			m_GlobalRenderResource._ibl_resource._specular_texture_image_view,
			m_GlobalRenderResource._ibl_resource._specular_texture_image_allocation,
			specular_maps[0]->m_width,
			specular_maps[0]->m_height,
			{ specular_maps[0]->m_pixels,
			 specular_maps[1]->m_pixels,
			 specular_maps[2]->m_pixels,
			 specular_maps[3]->m_pixels,
			 specular_maps[4]->m_pixels,
			 specular_maps[5]->m_pixels },
			specular_maps[0]->m_format,
			specular_cubemap_miplevels);
	}

	Ref<TextureData> RenderSource::LoadTextureHDR(std::string file, int desired_channels)
	{
		Ref<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
		AS_CORE_ASSERT(asset_manager);

		Ref<TextureData> texture = std::make_shared<TextureData>();

		int iw, ih, n;
		texture->m_pixels =
			stbi_loadf(asset_manager->getFullPath(file).generic_string().c_str(), &iw, &ih, &n, desired_channels);

		if (!texture->m_pixels)
			return nullptr;

		texture->m_width = iw;
		texture->m_height = ih;
		switch (desired_channels)
		{
		case 2:
			texture->m_format = RHIFormat::RHI_FORMAT_R32G32_SFLOAT;
			break;
		case 4:
			texture->m_format = RHIFormat::RHI_FORMAT_R32G32B32A32_SFLOAT;
			break;
		default:
			// three component format is not supported in some vulkan driver implementations
			throw std::runtime_error("unsupported channels number");
			break;
		}
		texture->m_depth = 1;
		texture->m_array_layers = 1;
		texture->m_mip_levels = 1;
		texture->m_type = ASTAN_IMAGE_TYPE::ASTAN_IMAGE_TYPE_2D;

		return texture;
	}

	Ref<TextureData> RenderSource::LoadTexture(std::string file, bool is_srgb)
	{
		Ref<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
		AS_CORE_ASSERT(asset_manager);

		Ref<TextureData> texture = std::make_shared<TextureData>();

		int iw, ih, n;
		texture->m_pixels = stbi_load(asset_manager->getFullPath(file).generic_string().c_str(), &iw, &ih, &n, 4);

		if (!texture->m_pixels)
			return nullptr;

		texture->m_width = iw;
		texture->m_height = ih;
		texture->m_format = (is_srgb) ? RHIFormat::RHI_FORMAT_R8G8B8A8_SRGB :
			RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
		texture->m_depth = 1;
		texture->m_array_layers = 1;
		texture->m_mip_levels = 1;
		texture->m_type = ASTAN_IMAGE_TYPE::ASTAN_IMAGE_TYPE_2D;

		return texture;
	}

	void RenderSource::UpdatePerFrameBuffer(Scene* scene,Ref<EditorCamera> camera)
	{
		glm::mat4 view_matrix = camera->GetViewMatrix();
		glm::mat4 proj_matrix = camera->GetProjection();
		glm::vec3 camera_position = camera->GetPosition();
		glm::mat4 proj_view_matrix = proj_matrix * view_matrix;

		// ambient light
		auto view = scene->GetAllEntitiesWith<AmbientLightComponent>();
		for (auto e : view)
		{
			Entity entity = { e, scene };

			auto& ambinetLgiht = entity.GetComponent<AmbientLightComponent>();

			m_MeshPerframeStorageBufferObject.proj_view_matrix = proj_view_matrix;
			m_MeshPerframeStorageBufferObject.camera_position = camera_position;
			m_MeshPerframeStorageBufferObject.ambient_light = ambinetLgiht.Irradiance;
		}


		auto view = scene->GetAllEntitiesWith<PointLightComponent>();
		m_MeshPointLightShadowPerframeStorageBufferObject.point_light_num = 0;
		for (auto e : view)
		{
			Entity entity = { e, scene };
			auto& ambinetLgiht = entity.GetComponent<PointLightComponent>();
			m_MeshPointLightShadowPerframeStorageBufferObject.point_light_num++;
		}


		// set ubo data
		m_ParticleCollisionPerframeStorageBufferObject.view_matrix = view_matrix;
		m_ParticleCollisionPerframeStorageBufferObject.proj_view_matrix = proj_view_matrix;
		m_ParticleCollisionPerframeStorageBufferObject.proj_inv_matrix = glm::inverse(proj_matrix);

		// point lights
		{
			auto view = scene->GetAllEntitiesWith<PointLightComponent>();
			int j = 0;
			for (auto e : view)
			{

				Entity entity = { e, scene };
				auto& ambinetLgiht = entity.GetComponent<PointLightComponent>();
				glm::vec3 point_light_position = ambinetLgiht.Position;
				glm::vec3 point_light_intensity = glm::vec3(ambinetLgiht.Fulx.x / (4.0f * PI), ambinetLgiht.Fulx.y / (4.0f * PI), ambinetLgiht.Fulx.z / (4.0f * PI));

				float radius = ambinetLgiht.calculteRadius();

				m_MeshPerframeStorageBufferObject.scene_point_lights[j].position = point_light_position;
				m_MeshPerframeStorageBufferObject.scene_point_lights[j].radius = radius;
				m_MeshPerframeStorageBufferObject.scene_point_lights[j].intensity = point_light_intensity;
				m_MeshPointLightShadowPerframeStorageBufferObject.point_lights_position_and_radius[j] = glm::vec4(point_light_position, radius);
				j++;
			}
		}

		// directional light
		{
			auto view = scene->GetAllEntitiesWith<PDirectionalLightComponent>();
			for (auto e : view)
			{

				Entity entity = { e, scene };
				auto& DirectionalLight = entity.GetComponent<PDirectionalLightComponent>();
				m_MeshPerframeStorageBufferObject.scene_directional_light.direction = glm::normalize(DirectionalLight.Direction);
				m_MeshPerframeStorageBufferObject.scene_directional_light.color = DirectionalLight.Color;
			}
		}

		// pick pass view projection matrix
		m_MeshInefficientPickPerframeStorageBufferObject.proj_view_matrix = proj_view_matrix;

		m_ParticlebillboardPerframeStorageBufferObject.proj_view_matrix = proj_view_matrix;

		//TODO
		m_ParticlebillboardPerframeStorageBufferObject.right_direction = camera->GetRightDirection();
		m_ParticlebillboardPerframeStorageBufferObject.foward_direction = camera->GetForwardDirection();
		m_ParticlebillboardPerframeStorageBufferObject.up_direction = camera->GetUpDirection();
	}

	void RenderSource::ResetRingBufferOffset(uint8_t current_frame_index)
	{
		m_GlobalRenderResource._storage_buffer._global_upload_ringbuffers_end[current_frame_index] =
			m_GlobalRenderResource._storage_buffer._global_upload_ringbuffers_begin[current_frame_index];
	}
}