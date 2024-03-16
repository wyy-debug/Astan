#include "aspch.h"
#include "RenderSource.h"
#include "RenderUtils.h"
#include "Astan/Scene/Scene.h"
#include "Astan/Scene/Entity.h"
#include "Platform/Vulkan/VulkanUtil.h"
#include "RenderMesh.h"
#include <Astan/Core/AssetManager.h>
#include <stb_image.h>
#include "../../../../MeshData.h"
#include <Astan/Renderer/NewSystem/RenderPass.h>


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

		{
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
		//Ref<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
		Ref<AssetManager> asset_manager = CreateRef<AssetManager>();
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
		//Ref<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
		Ref<AssetManager> asset_manager = CreateRef<AssetManager>();
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
	
	VulkanMesh& RenderSource::GetorCreateVulkanMesh(Ref<VulkanRendererAPI> rhi, RenderEntityComponent entity, RenderMeshData data)
	{
		size_t assetid = entity.m_MeshAssetId;

        auto it = m_VulkanMeshes.find(assetid);
        if (it != m_VulkanMeshes.end())
        {
            return it->second;
        }
        else
        {
            VulkanMesh temp;
            auto       res = m_VulkanMeshes.insert(std::make_pair(assetid, std::move(temp)));
            assert(res.second);

            uint32_t index_buffer_size = static_cast<uint32_t>(data.m_static_mesh_data.m_index_buffer->m_size);
            void*    index_buffer_data = data.m_static_mesh_data.m_index_buffer->m_data;

            uint32_t vertex_buffer_size = static_cast<uint32_t>(data.m_static_mesh_data.m_vertex_buffer->m_size);
            MeshVertexDataDefinition* vertex_buffer_data =
                reinterpret_cast<MeshVertexDataDefinition*>(data.m_static_mesh_data.m_vertex_buffer->m_data);

            VulkanMesh& now_mesh = res.first->second;

            if (data.m_skeleton_binding_buffer)
            {
                uint32_t joint_binding_buffer_size = (uint32_t)data.m_skeleton_binding_buffer->m_size;
                MeshVertexBindingDataDefinition* joint_binding_buffer_data =
                    reinterpret_cast<MeshVertexBindingDataDefinition*>(data.m_skeleton_binding_buffer->m_data);
                UpdateMeshData(rhi,
                               true,
                               index_buffer_size,
                               index_buffer_data,
                               vertex_buffer_size,
                               vertex_buffer_data,
                               joint_binding_buffer_size,
                               joint_binding_buffer_data,
                               now_mesh);
            }
            else
            {
                UpdateMeshData(rhi,
                               false,
                               index_buffer_size,
                               index_buffer_data,
                               vertex_buffer_size,
                               vertex_buffer_data,
                               0,
                               NULL,
                               now_mesh);
            }

            return now_mesh;
        }
	}
	
	VulkanPBRMaterial& RenderSource::GetorCreateVulkanMesh(Ref<VulkanRendererAPI> rhi, RenderEntityComponent entity, RenderMaterialData data)
	{
		VulkanRendererAPI* vulkan_context = static_cast<VulkanRendererAPI*>(rhi.get());

		size_t assetid = entity.m_MaterialAssetId;

		auto it = m_vulkan_pbr_materials.find(assetid);
		if (it != m_vulkan_pbr_materials.end())
		{
			return it->second;
		}
		else
		{
			VulkanPBRMaterial temp;
			auto              res = m_vulkan_pbr_materials.insert(std::make_pair(assetid, std::move(temp)));
			assert(res.second);

			float empty_image[] = { 0.5f, 0.5f, 0.5f, 0.5f };

			void* base_color_image_pixels = empty_image;
			uint32_t           base_color_image_width = 1;
			uint32_t           base_color_image_height = 1;
			RHIFormat base_color_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_SRGB;
			if (data.m_base_color_texture)
			{
				base_color_image_pixels = data.m_base_color_texture->m_pixels;
				base_color_image_width = static_cast<uint32_t>(data.m_base_color_texture->m_width);
				base_color_image_height = static_cast<uint32_t>(data.m_base_color_texture->m_height);
				base_color_image_format = data.m_base_color_texture->m_format;
			}

			void* metallic_roughness_image_pixels = empty_image;
			uint32_t           metallic_roughness_width = 1;
			uint32_t           metallic_roughness_height = 1;
			RHIFormat metallic_roughness_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
			if (data.m_metallic_roughness_texture)
			{
				metallic_roughness_image_pixels = data.m_metallic_roughness_texture->m_pixels;
				metallic_roughness_width = static_cast<uint32_t>(data.m_metallic_roughness_texture->m_width);
				metallic_roughness_height = static_cast<uint32_t>(data.m_metallic_roughness_texture->m_height);
				metallic_roughness_format = data.m_metallic_roughness_texture->m_format;
			}

			void* normal_roughness_image_pixels = empty_image;
			uint32_t           normal_roughness_width = 1;
			uint32_t           normal_roughness_height = 1;
			RHIFormat normal_roughness_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
			if (data.m_normal_texture)
			{
				normal_roughness_image_pixels = data.m_normal_texture->m_pixels;
				normal_roughness_width = static_cast<uint32_t>(data.m_normal_texture->m_width);
				normal_roughness_height = static_cast<uint32_t>(data.m_normal_texture->m_height);
				normal_roughness_format = data.m_normal_texture->m_format;
			}

			void* occlusion_image_pixels = empty_image;
			uint32_t           occlusion_image_width = 1;
			uint32_t           occlusion_image_height = 1;
			RHIFormat occlusion_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
			if (data.m_occlusion_texture)
			{
				occlusion_image_pixels = data.m_occlusion_texture->m_pixels;
				occlusion_image_width = static_cast<uint32_t>(data.m_occlusion_texture->m_width);
				occlusion_image_height = static_cast<uint32_t>(data.m_occlusion_texture->m_height);
				occlusion_image_format = data.m_occlusion_texture->m_format;
			}

			void* emissive_image_pixels = empty_image;
			uint32_t           emissive_image_width = 1;
			uint32_t           emissive_image_height = 1;
			RHIFormat emissive_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
			if (data.m_emissive_texture)
			{
				emissive_image_pixels = data.m_emissive_texture->m_pixels;
				emissive_image_width = static_cast<uint32_t>(data.m_emissive_texture->m_width);
				emissive_image_height = static_cast<uint32_t>(data.m_emissive_texture->m_height);
				emissive_image_format = data.m_emissive_texture->m_format;
			}

			VulkanPBRMaterial& now_material = res.first->second;

			// similiarly to the vertex/index buffer, we should allocate the uniform
			// buffer in DEVICE_LOCAL memory and use the temp stage buffer to copy the
			// data
			{
				// temporary staging buffer

				RHIDeviceSize buffer_size = sizeof(MeshPerMaterialUniformBufferObject);

				RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
				RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
				rhi->CreateBuffer(
					buffer_size,
					RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
					RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					inefficient_staging_buffer,
					inefficient_staging_buffer_memory);
				// RHI_BUFFER_USAGE_TRANSFER_SRC_BIT: buffer can be used as source in a
				// memory transfer operation

				void* staging_buffer_data = nullptr;
				rhi->MapMemory(
					inefficient_staging_buffer_memory,
					0,
					buffer_size,
					0,
					&staging_buffer_data);

				MeshPerMaterialUniformBufferObject& material_uniform_buffer_info =
					(*static_cast<MeshPerMaterialUniformBufferObject*>(staging_buffer_data));
				material_uniform_buffer_info.is_blend = entity.m_Blend;
				material_uniform_buffer_info.is_double_sided = entity.m_DoubleSided;
				material_uniform_buffer_info.baseColorFactor = entity.m_BaseColorFactor;
				material_uniform_buffer_info.metallicFactor = entity.m_MetallicFactor;
				material_uniform_buffer_info.roughnessFactor = entity.m_RoughnessFactor;
				material_uniform_buffer_info.normalScale = entity.m_NormalScale;
				material_uniform_buffer_info.occlusionStrength = entity.m_OcclusionStrength;
				material_uniform_buffer_info.emissiveFactor = entity.m_EmissiveFactor;

				rhi->UnmapMemory(inefficient_staging_buffer_memory);

				// use the vmaAllocator to allocate asset uniform buffer
				RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				bufferInfo.size = buffer_size;
				bufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

				VmaAllocationCreateInfo allocInfo = {};
				allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

				rhi->CreateBufferWithAlignmentVMA(
					vulkan_context->m_assets_allocator,
					&bufferInfo,
					&allocInfo,
					m_GlobalRenderResource._storage_buffer._min_uniform_buffer_offset_alignment,
					now_material.material_uniform_buffer,
					&now_material.material_uniform_buffer_allocation,
					NULL);

				// use the data from staging buffer
				rhi->CopyBuffer(inefficient_staging_buffer, now_material.material_uniform_buffer, 0, 0, buffer_size);

				// release staging buffer
				rhi->DestroyBuffer(inefficient_staging_buffer);
				rhi->FreeMemory(inefficient_staging_buffer_memory);
			}

			TextureDataToUpdate update_texture_data;
			update_texture_data.base_color_image_pixels = base_color_image_pixels;
			update_texture_data.base_color_image_width = base_color_image_width;
			update_texture_data.base_color_image_height = base_color_image_height;
			update_texture_data.base_color_image_format = base_color_image_format;
			update_texture_data.metallic_roughness_image_pixels = metallic_roughness_image_pixels;
			update_texture_data.metallic_roughness_image_width = metallic_roughness_width;
			update_texture_data.metallic_roughness_image_height = metallic_roughness_height;
			update_texture_data.metallic_roughness_image_format = metallic_roughness_format;
			update_texture_data.normal_roughness_image_pixels = normal_roughness_image_pixels;
			update_texture_data.normal_roughness_image_width = normal_roughness_width;
			update_texture_data.normal_roughness_image_height = normal_roughness_height;
			update_texture_data.normal_roughness_image_format = normal_roughness_format;
			update_texture_data.occlusion_image_pixels = occlusion_image_pixels;
			update_texture_data.occlusion_image_width = occlusion_image_width;
			update_texture_data.occlusion_image_height = occlusion_image_height;
			update_texture_data.occlusion_image_format = occlusion_image_format;
			update_texture_data.emissive_image_pixels = emissive_image_pixels;
			update_texture_data.emissive_image_width = emissive_image_width;
			update_texture_data.emissive_image_height = emissive_image_height;
			update_texture_data.emissive_image_format = emissive_image_format;
			update_texture_data.now_material = &now_material;

			UpdateTextureImageData(rhi, update_texture_data);

			RHIDescriptorSetAllocateInfo material_descriptor_set_alloc_info;
			material_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			material_descriptor_set_alloc_info.pNext = NULL;
			material_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
			material_descriptor_set_alloc_info.descriptorSetCount = 1;
			material_descriptor_set_alloc_info.pSetLayouts = m_material_descriptor_set_layout;

			if (RHI_SUCCESS != rhi->AllocateDescriptorSets(
				&material_descriptor_set_alloc_info,
				now_material.material_descriptor_set))
			{
				throw std::runtime_error("allocate material descriptor set");
			}

			RHIDescriptorBufferInfo material_uniform_buffer_info = {};
			material_uniform_buffer_info.offset = 0;
			material_uniform_buffer_info.range = sizeof(MeshPerMaterialUniformBufferObject);
			material_uniform_buffer_info.buffer = now_material.material_uniform_buffer;

			RHIDescriptorImageInfo base_color_image_info = {};
			base_color_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			base_color_image_info.imageView = now_material.base_color_image_view;
			base_color_image_info.sampler = rhi->GetOrCreateMipmapSampler(base_color_image_width,
				base_color_image_height);

			RHIDescriptorImageInfo metallic_roughness_image_info = {};
			metallic_roughness_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			metallic_roughness_image_info.imageView = now_material.metallic_roughness_image_view;
			metallic_roughness_image_info.sampler = rhi->GetOrCreateMipmapSampler(metallic_roughness_width,
				metallic_roughness_height);

			RHIDescriptorImageInfo normal_roughness_image_info = {};
			normal_roughness_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			normal_roughness_image_info.imageView = now_material.normal_image_view;
			normal_roughness_image_info.sampler = rhi->GetOrCreateMipmapSampler(normal_roughness_width,
				normal_roughness_height);

			RHIDescriptorImageInfo occlusion_image_info = {};
			occlusion_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			occlusion_image_info.imageView = now_material.occlusion_image_view;
			occlusion_image_info.sampler = rhi->GetOrCreateMipmapSampler(occlusion_image_width, occlusion_image_height);

			RHIDescriptorImageInfo emissive_image_info = {};
			emissive_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			emissive_image_info.imageView = now_material.emissive_image_view;
			emissive_image_info.sampler = rhi->GetOrCreateMipmapSampler(emissive_image_width, emissive_image_height);

			RHIWriteDescriptorSet mesh_descriptor_writes_info[6];

			mesh_descriptor_writes_info[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			mesh_descriptor_writes_info[0].pNext = NULL;
			mesh_descriptor_writes_info[0].dstSet = now_material.material_descriptor_set;
			mesh_descriptor_writes_info[0].dstBinding = 0;
			mesh_descriptor_writes_info[0].dstArrayElement = 0;
			mesh_descriptor_writes_info[0].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			mesh_descriptor_writes_info[0].descriptorCount = 1;
			mesh_descriptor_writes_info[0].pBufferInfo = &material_uniform_buffer_info;

			mesh_descriptor_writes_info[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			mesh_descriptor_writes_info[1].pNext = NULL;
			mesh_descriptor_writes_info[1].dstSet = now_material.material_descriptor_set;
			mesh_descriptor_writes_info[1].dstBinding = 1;
			mesh_descriptor_writes_info[1].dstArrayElement = 0;
			mesh_descriptor_writes_info[1].descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			mesh_descriptor_writes_info[1].descriptorCount = 1;
			mesh_descriptor_writes_info[1].pImageInfo = &base_color_image_info;

			mesh_descriptor_writes_info[2] = mesh_descriptor_writes_info[1];
			mesh_descriptor_writes_info[2].dstBinding = 2;
			mesh_descriptor_writes_info[2].pImageInfo = &metallic_roughness_image_info;

			mesh_descriptor_writes_info[3] = mesh_descriptor_writes_info[1];
			mesh_descriptor_writes_info[3].dstBinding = 3;
			mesh_descriptor_writes_info[3].pImageInfo = &normal_roughness_image_info;

			mesh_descriptor_writes_info[4] = mesh_descriptor_writes_info[1];
			mesh_descriptor_writes_info[4].dstBinding = 4;
			mesh_descriptor_writes_info[4].pImageInfo = &occlusion_image_info;

			mesh_descriptor_writes_info[5] = mesh_descriptor_writes_info[1];
			mesh_descriptor_writes_info[5].dstBinding = 5;
			mesh_descriptor_writes_info[5].pImageInfo = &emissive_image_info;

			rhi->UpdateDescriptorSets(6, mesh_descriptor_writes_info, 0, nullptr);

			return now_material;
		}
	}

	void RenderSource::UpdatePerFrameBuffer(Scene* scene,Ref<EditorCamera> camera)
	{
		glm::mat4 view_matrix = camera->GetViewMatrix();
		glm::mat4 proj_matrix = camera->GetProjection();
		glm::vec3 camera_position = camera->GetPosition();
		glm::mat4 proj_view_matrix = proj_matrix * view_matrix;

		// ambient light
		{
			auto view = scene->GetAllEntitiesWith<AmbientLightComponent>();
			for (auto e : view)
			{
				Entity entity = { e, scene };

				auto& ambinetLgiht = entity.GetComponent<AmbientLightComponent>();

				m_MeshPerframeStorageBufferObject.proj_view_matrix = proj_view_matrix;
				m_MeshPerframeStorageBufferObject.camera_position = camera_position;
				m_MeshPerframeStorageBufferObject.ambient_light = ambinetLgiht.Irradiance;
			}
		}

		{
			auto view = scene->GetAllEntitiesWith<PointLightComponent>();
			m_MeshPointLightShadowPerframeStorageBufferObject.point_light_num = 0;
			for (auto e : view)
			{
				Entity entity = { e, scene };
				auto& ambinetLgiht = entity.GetComponent<PointLightComponent>();
				m_MeshPointLightShadowPerframeStorageBufferObject.point_light_num++;
			}
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

	void RenderSource::UploadGameObjectRenderResource(Ref<VulkanRendererAPI> rhi, RenderEntityComponent entity, RenderMeshData meshData,RenderMaterialData materialData)
	{
		GetorCreateVulkanMesh(rhi, entity, meshData);
		GetorCreateVulkanMesh(rhi, entity, materialData);
	}

	void RenderSource::UploadGameObjectRenderResource(Ref<VulkanRendererAPI> rhi, RenderEntityComponent entity, RenderMeshData data)
	{
		GetorCreateVulkanMesh(rhi, entity, data);
	}

	void RenderSource::UploadGameObjectRenderResource(Ref<VulkanRendererAPI> rhi, RenderEntityComponent entity, RenderMaterialData data)
	{
		GetorCreateVulkanMesh(rhi, entity, data);
	}		

	RenderMeshData RenderSource::LoadMeshData(const std::string meshfilepath, AxisAlignedBox& boundingBox)
	{
		//std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
		Ref<AssetManager> asset_manager = CreateRef<AssetManager>();
		AS_CORE_ASSERT(asset_manager);

		RenderMeshData ret;

		if (std::filesystem::path(meshfilepath).extension() == ".obj")
		{
			ret.m_static_mesh_data = loadStaticMesh(meshfilepath, boundingBox);
		}
		else if (std::filesystem::path(meshfilepath).extension() == ".json")
		{
			std::shared_ptr<MeshData> bind_data = std::make_shared<MeshData>();
			asset_manager->loadAsset<MeshData>(meshfilepath, *bind_data);

			// vertex buffer
			size_t vertex_size = bind_data->vertex_buffer.size() * sizeof(MeshVertexDataDefinition);
			ret.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(vertex_size);
			MeshVertexDataDefinition* vertex =
				(MeshVertexDataDefinition*)ret.m_static_mesh_data.m_vertex_buffer->m_data;
			for (size_t i = 0; i < bind_data->vertex_buffer.size(); i++)
			{
				vertex[i].x = bind_data->vertex_buffer[i].px;
				vertex[i].y = bind_data->vertex_buffer[i].py;
				vertex[i].z = bind_data->vertex_buffer[i].pz;
				vertex[i].nx = bind_data->vertex_buffer[i].nx;
				vertex[i].ny = bind_data->vertex_buffer[i].ny;
				vertex[i].nz = bind_data->vertex_buffer[i].nz;
				vertex[i].tx = bind_data->vertex_buffer[i].tx;
				vertex[i].ty = bind_data->vertex_buffer[i].ty;
				vertex[i].tz = bind_data->vertex_buffer[i].tz;
				vertex[i].u = bind_data->vertex_buffer[i].u;
				vertex[i].v = bind_data->vertex_buffer[i].v;

				boundingBox.Merge(glm::vec3(vertex[i].x, vertex[i].y, vertex[i].z));
			}

			// index buffer
			size_t index_size = bind_data->index_buffer.size() * sizeof(uint16_t);
			ret.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(index_size);
			uint16_t* index = (uint16_t*)ret.m_static_mesh_data.m_index_buffer->m_data;
			for (size_t i = 0; i < bind_data->index_buffer.size(); i++)
			{
				index[i] = static_cast<uint16_t>(bind_data->index_buffer[i]);
			}

			// skeleton binding buffer
			size_t data_size = bind_data->bind.size() * sizeof(MeshVertexBindingDataDefinition);
			ret.m_skeleton_binding_buffer = std::make_shared<BufferData>(data_size);
			MeshVertexBindingDataDefinition* binding_data =
				reinterpret_cast<MeshVertexBindingDataDefinition*>(ret.m_skeleton_binding_buffer->m_data);
			for (size_t i = 0; i < bind_data->bind.size(); i++)
			{
				binding_data[i].m_index0 = bind_data->bind[i].index0;
				binding_data[i].m_index1 = bind_data->bind[i].index1;
				binding_data[i].m_index2 = bind_data->bind[i].index2;
				binding_data[i].m_index3 = bind_data->bind[i].index3;
				binding_data[i].m_weight0 = bind_data->bind[i].weight0;
				binding_data[i].m_weight1 = bind_data->bind[i].weight1;
				binding_data[i].m_weight2 = bind_data->bind[i].weight2;
				binding_data[i].m_weight3 = bind_data->bind[i].weight3;
			}
		}

		m_bounding_box_cache_map.insert(std::make_pair(meshfilepath, boundingBox));

		return ret;
	}

	RenderMaterialData RenderSource::LoadMaterialData(const MaterialSourceDesc& source)
	{
		RenderMaterialData ret;
        ret.m_base_color_texture         = LoadTexture(source.m_base_color_file, true);
        ret.m_metallic_roughness_texture = LoadTexture(source.m_metallic_roughness_file);
        ret.m_normal_texture             = LoadTexture(source.m_normal_file);
        ret.m_occlusion_texture          = LoadTexture(source.m_occlusion_file);
        ret.m_emissive_texture           = LoadTexture(source.m_emissive_file);
        return ret;
	}


	AxisAlignedBox RenderSource::GetCachedBoudingBox(const std::string meshfilepath) const
	{

		auto find_it = m_bounding_box_cache_map.find(meshfilepath);
		if (find_it != m_bounding_box_cache_map.end())
		{
			return find_it->second;
		}
		return AxisAlignedBox();
	}

	void RenderSource::ResetRingBufferOffset(uint8_t current_frame_index)
	{
		m_GlobalRenderResource._storage_buffer._global_upload_ringbuffers_end[current_frame_index] =
			m_GlobalRenderResource._storage_buffer._global_upload_ringbuffers_begin[current_frame_index];
	}
	void RenderSource::UpdateMeshData(Ref<VulkanRendererAPI> rhi, bool enable_vertex_blending, uint32_t index_buffer_size, void* index_buffer_data, uint32_t vertex_buffer_size, MeshVertexDataDefinition const* vertex_buffer_data, uint32_t joint_binding_buffer_size, MeshVertexBindingDataDefinition const* joint_binding_buffer_data, VulkanMesh& now_mesh)
	{
		now_mesh.enable_vertex_blending = enable_vertex_blending;
		assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
		now_mesh.mesh_vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);
		UpdateVertexBuffer(rhi,
			enable_vertex_blending,
			vertex_buffer_size,
			vertex_buffer_data,
			joint_binding_buffer_size,
			joint_binding_buffer_data,
			index_buffer_size,
			reinterpret_cast<uint16_t*>(index_buffer_data),
			now_mesh);
		assert(0 == (index_buffer_size % sizeof(uint16_t)));
		now_mesh.mesh_index_count = index_buffer_size / sizeof(uint16_t);
		UpdateIndexBuffer(rhi, index_buffer_size, index_buffer_data, now_mesh);
	}
	void RenderSource::UpdateVertexBuffer(Ref<VulkanRendererAPI> rhi, bool enable_vertex_blending, uint32_t vertex_buffer_size, MeshVertexDataDefinition const* vertex_buffer_data, uint32_t joint_binding_buffer_size, MeshVertexBindingDataDefinition const* joint_binding_buffer_data, uint32_t index_buffer_size, uint16_t* index_buffer_data, VulkanMesh& now_mesh)
	{
		VulkanRendererAPI* vulkan_context = static_cast<VulkanRendererAPI*>(rhi.get());

		if (enable_vertex_blending)
		{
			assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
			uint32_t vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);
			assert(0 == (index_buffer_size % sizeof(uint16_t)));
			uint32_t index_count = index_buffer_size / sizeof(uint16_t);

			RHIDeviceSize vertex_position_buffer_size = sizeof(MeshVertex::VulkanMeshVertexPosition) * vertex_count;
			RHIDeviceSize vertex_varying_enable_blending_buffer_size =
				sizeof(MeshVertex::VulkanMeshVertexVaringEnableBlending) * vertex_count;
			RHIDeviceSize vertex_varying_buffer_size = sizeof(MeshVertex::VulkanMeshVertexVaring) * vertex_count;
			RHIDeviceSize vertex_joint_binding_buffer_size =
				sizeof(MeshVertex::VulkanMeshVertexJointBinding) * index_count;

			RHIDeviceSize vertex_position_buffer_offset = 0;
			RHIDeviceSize vertex_varying_enable_blending_buffer_offset =
				vertex_position_buffer_offset + vertex_position_buffer_size;
			RHIDeviceSize vertex_varying_buffer_offset =
				vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;
			RHIDeviceSize vertex_joint_binding_buffer_offset = vertex_varying_buffer_offset + vertex_varying_buffer_size;

			// temporary staging buffer
			RHIDeviceSize inefficient_staging_buffer_size =
				vertex_position_buffer_size + vertex_varying_enable_blending_buffer_size + vertex_varying_buffer_size +
				vertex_joint_binding_buffer_size;
			RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
			RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
			rhi->CreateBuffer(inefficient_staging_buffer_size,
				RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
				RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				inefficient_staging_buffer,
				inefficient_staging_buffer_memory);

			void* inefficient_staging_buffer_data;
			rhi->MapMemory(inefficient_staging_buffer_memory,
				0,
				RHI_WHOLE_SIZE,
				0,
				&inefficient_staging_buffer_data);

			MeshVertex::VulkanMeshVertexPosition* mesh_vertex_positions =
				reinterpret_cast<MeshVertex::VulkanMeshVertexPosition*>(
					reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_position_buffer_offset);
			MeshVertex::VulkanMeshVertexVaringEnableBlending* mesh_vertex_blending_varyings =
				reinterpret_cast<MeshVertex::VulkanMeshVertexVaringEnableBlending*>(
					reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
					vertex_varying_enable_blending_buffer_offset);
			MeshVertex::VulkanMeshVertexVaring* mesh_vertex_varyings =
				reinterpret_cast<MeshVertex::VulkanMeshVertexVaring*>(
					reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_varying_buffer_offset);
			MeshVertex::VulkanMeshVertexJointBinding* mesh_vertex_joint_binding =
				reinterpret_cast<MeshVertex::VulkanMeshVertexJointBinding*>(
					reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_joint_binding_buffer_offset);

			for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
			{
				glm::vec3 normal = glm::vec3(vertex_buffer_data[vertex_index].nx,
					vertex_buffer_data[vertex_index].ny,
					vertex_buffer_data[vertex_index].nz);
				glm::vec3 tangent = glm::vec3(vertex_buffer_data[vertex_index].tx,
					vertex_buffer_data[vertex_index].ty,
					vertex_buffer_data[vertex_index].tz);

				mesh_vertex_positions[vertex_index].position = glm::vec3(vertex_buffer_data[vertex_index].x,
					vertex_buffer_data[vertex_index].y,
					vertex_buffer_data[vertex_index].z);

				mesh_vertex_blending_varyings[vertex_index].normal = normal;
				mesh_vertex_blending_varyings[vertex_index].tangent = tangent;

				mesh_vertex_varyings[vertex_index].texcoord =
					glm::vec2(vertex_buffer_data[vertex_index].u, vertex_buffer_data[vertex_index].v);
			}

			for (uint32_t index_index = 0; index_index < index_count; ++index_index)
			{
				uint32_t vertex_buffer_index = index_buffer_data[index_index];

				// TODO: move to assets loading process

				mesh_vertex_joint_binding[index_index].indices[0] = joint_binding_buffer_data[vertex_buffer_index].m_index0;
				mesh_vertex_joint_binding[index_index].indices[1] = joint_binding_buffer_data[vertex_buffer_index].m_index1;
				mesh_vertex_joint_binding[index_index].indices[2] = joint_binding_buffer_data[vertex_buffer_index].m_index2;
				mesh_vertex_joint_binding[index_index].indices[3] = joint_binding_buffer_data[vertex_buffer_index].m_index3;

				float inv_total_weight = joint_binding_buffer_data[vertex_buffer_index].m_weight0 +
					joint_binding_buffer_data[vertex_buffer_index].m_weight1 +
					joint_binding_buffer_data[vertex_buffer_index].m_weight2 +
					joint_binding_buffer_data[vertex_buffer_index].m_weight3;

				inv_total_weight = (inv_total_weight != 0.0) ? 1 / inv_total_weight : 1.0;

				mesh_vertex_joint_binding[index_index].weights =
					glm::vec4(joint_binding_buffer_data[vertex_buffer_index].m_weight0 * inv_total_weight,
						joint_binding_buffer_data[vertex_buffer_index].m_weight1 * inv_total_weight,
						joint_binding_buffer_data[vertex_buffer_index].m_weight2 * inv_total_weight,
						joint_binding_buffer_data[vertex_buffer_index].m_weight3 * inv_total_weight);
			}

			rhi->UnmapMemory(inefficient_staging_buffer_memory);

			// use the vmaAllocator to allocate asset vertex buffer
			RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			bufferInfo.usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferInfo.size = vertex_position_buffer_size;
			rhi->CreateBufferVMA(vulkan_context->m_assets_allocator,
				&bufferInfo,
				&allocInfo,
				now_mesh.mesh_vertex_position_buffer,
				&now_mesh.mesh_vertex_position_buffer_allocation,
				NULL);
			bufferInfo.size = vertex_varying_enable_blending_buffer_size;
			rhi->CreateBufferVMA(vulkan_context->m_assets_allocator,
				&bufferInfo,
				&allocInfo,
				now_mesh.mesh_vertex_varying_enable_blending_buffer,
				&now_mesh.mesh_vertex_varying_enable_blending_buffer_allocation,
				NULL);
			bufferInfo.size = vertex_varying_buffer_size;
			rhi->CreateBufferVMA(vulkan_context->m_assets_allocator,
				&bufferInfo,
				&allocInfo,
				now_mesh.mesh_vertex_varying_buffer,
				&now_mesh.mesh_vertex_varying_buffer_allocation,
				NULL);

			bufferInfo.usage = RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferInfo.size = vertex_joint_binding_buffer_size;
			rhi->CreateBufferVMA(vulkan_context->m_assets_allocator,
				&bufferInfo,
				&allocInfo,
				now_mesh.mesh_vertex_joint_binding_buffer,
				&now_mesh.mesh_vertex_joint_binding_buffer_allocation,
				NULL);

			// use the data from staging buffer
			rhi->CopyBuffer(inefficient_staging_buffer,
				now_mesh.mesh_vertex_position_buffer,
				vertex_position_buffer_offset,
				0,
				vertex_position_buffer_size);
			rhi->CopyBuffer(inefficient_staging_buffer,
				now_mesh.mesh_vertex_varying_enable_blending_buffer,
				vertex_varying_enable_blending_buffer_offset,
				0,
				vertex_varying_enable_blending_buffer_size);
			rhi->CopyBuffer(inefficient_staging_buffer,
				now_mesh.mesh_vertex_varying_buffer,
				vertex_varying_buffer_offset,
				0,
				vertex_varying_buffer_size);
			rhi->CopyBuffer(inefficient_staging_buffer,
				now_mesh.mesh_vertex_joint_binding_buffer,
				vertex_joint_binding_buffer_offset,
				0,
				vertex_joint_binding_buffer_size);

			// release staging buffer
			rhi->DestroyBuffer(inefficient_staging_buffer);
			rhi->FreeMemory(inefficient_staging_buffer_memory);

			// update descriptor set
			RHIDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType =
				RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext = NULL;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts = m_mesh_descriptor_set_layout;

			if (RHI_SUCCESS != rhi->AllocateDescriptorSets(
				&mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
				now_mesh.mesh_vertex_blending_descriptor_set))
			{
				throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
			}

			RHIDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
			mesh_vertex_Joint_binding_storage_buffer_info.offset = 0;
			mesh_vertex_Joint_binding_storage_buffer_info.range = vertex_joint_binding_buffer_size;
			mesh_vertex_Joint_binding_storage_buffer_info.buffer = now_mesh.mesh_vertex_joint_binding_buffer;
			assert(mesh_vertex_Joint_binding_storage_buffer_info.range < m_GlobalRenderResource._storage_buffer._max_storage_buffer_range);

			RHIDescriptorSet* descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

			RHIWriteDescriptorSet descriptor_writes[1];

			RHIWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
				descriptor_writes[0];
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
				RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext = NULL;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet = descriptor_set_to_write;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding = 0;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
				RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
				&mesh_vertex_Joint_binding_storage_buffer_info;

			rhi->UpdateDescriptorSets((sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
				descriptor_writes,
				0,
				NULL);
		}
		else
		{
			AS_CORE_ASSERT(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
			uint32_t vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);

			RHIDeviceSize vertex_position_buffer_size = sizeof(MeshVertex::VulkanMeshVertexPosition) * vertex_count;
			RHIDeviceSize vertex_varying_enable_blending_buffer_size =
				sizeof(MeshVertex::VulkanMeshVertexVaringEnableBlending) * vertex_count;
			RHIDeviceSize vertex_varying_buffer_size = sizeof(MeshVertex::VulkanMeshVertexVaring) * vertex_count;

			RHIDeviceSize vertex_position_buffer_offset = 0;
			RHIDeviceSize vertex_varying_enable_blending_buffer_offset =
				vertex_position_buffer_offset + vertex_position_buffer_size;
			RHIDeviceSize vertex_varying_buffer_offset =
				vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;

			// temporary staging buffer
			RHIDeviceSize inefficient_staging_buffer_size =
				vertex_position_buffer_size + vertex_varying_enable_blending_buffer_size + vertex_varying_buffer_size;
			RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
			RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
			rhi->CreateBuffer(inefficient_staging_buffer_size,
				RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
				RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				inefficient_staging_buffer,
				inefficient_staging_buffer_memory);

			void* inefficient_staging_buffer_data;
			rhi->MapMemory(inefficient_staging_buffer_memory,
				0,
				RHI_WHOLE_SIZE,
				0,
				&inefficient_staging_buffer_data);

			MeshVertex::VulkanMeshVertexPosition* mesh_vertex_positions =
				reinterpret_cast<MeshVertex::VulkanMeshVertexPosition*>(
					reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_position_buffer_offset);
			MeshVertex::VulkanMeshVertexVaringEnableBlending* mesh_vertex_blending_varyings =
				reinterpret_cast<MeshVertex::VulkanMeshVertexVaringEnableBlending*>(
					reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
					vertex_varying_enable_blending_buffer_offset);
			MeshVertex::VulkanMeshVertexVaring* mesh_vertex_varyings =
				reinterpret_cast<MeshVertex::VulkanMeshVertexVaring*>(
					reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_varying_buffer_offset);

			for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
			{
				glm::vec3 normal = glm::vec3(vertex_buffer_data[vertex_index].nx,
					vertex_buffer_data[vertex_index].ny,
					vertex_buffer_data[vertex_index].nz);
				glm::vec3 tangent = glm::vec3(vertex_buffer_data[vertex_index].tx,
					vertex_buffer_data[vertex_index].ty,
					vertex_buffer_data[vertex_index].tz);

				mesh_vertex_positions[vertex_index].position = glm::vec3(vertex_buffer_data[vertex_index].x,
					vertex_buffer_data[vertex_index].y,
					vertex_buffer_data[vertex_index].z);

				mesh_vertex_blending_varyings[vertex_index].normal = normal;
				mesh_vertex_blending_varyings[vertex_index].tangent = tangent;

				mesh_vertex_varyings[vertex_index].texcoord =
					glm::vec2(vertex_buffer_data[vertex_index].u, vertex_buffer_data[vertex_index].v);
			}

			rhi->UnmapMemory(inefficient_staging_buffer_memory);

			// use the vmaAllocator to allocate asset vertex buffer
			RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			bufferInfo.size = vertex_position_buffer_size;
			rhi->CreateBufferVMA(vulkan_context->m_assets_allocator,
				&bufferInfo,
				&allocInfo,
				now_mesh.mesh_vertex_position_buffer,
				&now_mesh.mesh_vertex_position_buffer_allocation,
				NULL);
			bufferInfo.size = vertex_varying_enable_blending_buffer_size;
			rhi->CreateBufferVMA(vulkan_context->m_assets_allocator,
				&bufferInfo,
				&allocInfo,
				now_mesh.mesh_vertex_varying_enable_blending_buffer,
				&now_mesh.mesh_vertex_varying_enable_blending_buffer_allocation,
				NULL);
			bufferInfo.size = vertex_varying_buffer_size;
			rhi->CreateBufferVMA(vulkan_context->m_assets_allocator,
				&bufferInfo,
				&allocInfo,
				now_mesh.mesh_vertex_varying_buffer,
				&now_mesh.mesh_vertex_varying_buffer_allocation,
				NULL);

			// use the data from staging buffer
			rhi->CopyBuffer(inefficient_staging_buffer,
				now_mesh.mesh_vertex_position_buffer,
				vertex_position_buffer_offset,
				0,
				vertex_position_buffer_size);
			rhi->CopyBuffer(inefficient_staging_buffer,
				now_mesh.mesh_vertex_varying_enable_blending_buffer,
				vertex_varying_enable_blending_buffer_offset,
				0,
				vertex_varying_enable_blending_buffer_size);
			rhi->CopyBuffer(inefficient_staging_buffer,
				now_mesh.mesh_vertex_varying_buffer,
				vertex_varying_buffer_offset,
				0,
				vertex_varying_buffer_size);

			// release staging buffer
			rhi->DestroyBuffer(inefficient_staging_buffer);
			rhi->FreeMemory(inefficient_staging_buffer_memory);

			// update descriptor set
			RHIDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType =
				RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext = NULL;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
			mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts = m_mesh_descriptor_set_layout;

			if (RHI_SUCCESS != rhi->AllocateDescriptorSets(
				&mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
				now_mesh.mesh_vertex_blending_descriptor_set))
			{
				throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
			}

			RHIDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
			mesh_vertex_Joint_binding_storage_buffer_info.offset = 0;
			mesh_vertex_Joint_binding_storage_buffer_info.range = 1;
			mesh_vertex_Joint_binding_storage_buffer_info.buffer =
				m_GlobalRenderResource._storage_buffer._global_null_descriptor_storage_buffer;
			assert(mesh_vertex_Joint_binding_storage_buffer_info.range <
				m_GlobalRenderResource._storage_buffer._max_storage_buffer_range);

			RHIDescriptorSet* descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

			RHIWriteDescriptorSet descriptor_writes[1];

			RHIWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
				descriptor_writes[0];
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
				RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext = NULL;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet = descriptor_set_to_write;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding = 0;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
				RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
			mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
				&mesh_vertex_Joint_binding_storage_buffer_info;

			rhi->UpdateDescriptorSets((sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
				descriptor_writes,
				0,
				NULL);
		}
	}
	
	void RenderSource::UpdateIndexBuffer(Ref<VulkanRendererAPI> rhi, uint32_t index_buffer_size, void* index_buffer_data, VulkanMesh& now_mesh)
	{
		VulkanRendererAPI* vulkanRendererAPI = static_cast<VulkanRendererAPI*>(rhi.get());

		// temp staging buffer
		RHIDeviceSize buffer_size = index_buffer_size;

		RHIBuffer* inefficient_staging_buffer;
		RHIDeviceMemory* inefficient_staging_buffer_memory;
		rhi->CreateBuffer(buffer_size,
			RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			inefficient_staging_buffer,
			inefficient_staging_buffer_memory);

		void* staging_buffer_data;
		rhi->MapMemory(inefficient_staging_buffer_memory, 0, buffer_size, 0, &staging_buffer_data);
		memcpy(staging_buffer_data, index_buffer_data, (size_t)buffer_size);
		rhi->UnmapMemory(inefficient_staging_buffer_memory);

		// use the vmaAllocator to allocate asset index buffer
		RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = buffer_size;
		bufferInfo.usage = RHI_BUFFER_USAGE_INDEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		rhi->CreateBufferVMA(vulkanRendererAPI->m_assets_allocator,
			&bufferInfo,
			&allocInfo,
			now_mesh.mesh_index_buffer,
			&now_mesh.mesh_index_buffer_allocation,
			NULL);

		// use the data from staging buffer
		rhi->CopyBuffer(inefficient_staging_buffer, now_mesh.mesh_index_buffer, 0, 0, buffer_size);

		// release temp staging buffer
		rhi->DestroyBuffer(inefficient_staging_buffer);
		rhi->FreeMemory(inefficient_staging_buffer_memory);
	}

	void RenderSource::UpdateTextureImageData(Ref<VulkanRendererAPI> rhi, const TextureDataToUpdate& texture_data)
	{
		rhi->CreateGlobalImage(
			texture_data.now_material->base_color_texture_image,
			texture_data.now_material->base_color_image_view,
			texture_data.now_material->base_color_image_allocation,
			texture_data.base_color_image_width,
			texture_data.base_color_image_height,
			texture_data.base_color_image_pixels,
			texture_data.base_color_image_format);

		rhi->CreateGlobalImage(
			texture_data.now_material->metallic_roughness_texture_image,
			texture_data.now_material->metallic_roughness_image_view,
			texture_data.now_material->metallic_roughness_image_allocation,
			texture_data.metallic_roughness_image_width,
			texture_data.metallic_roughness_image_height,
			texture_data.metallic_roughness_image_pixels,
			texture_data.metallic_roughness_image_format);

		rhi->CreateGlobalImage(
			texture_data.now_material->normal_texture_image,
			texture_data.now_material->normal_image_view,
			texture_data.now_material->normal_image_allocation,
			texture_data.normal_roughness_image_width,
			texture_data.normal_roughness_image_height,
			texture_data.normal_roughness_image_pixels,
			texture_data.normal_roughness_image_format);

		rhi->CreateGlobalImage(
			texture_data.now_material->occlusion_texture_image,
			texture_data.now_material->occlusion_image_view,
			texture_data.now_material->occlusion_image_allocation,
			texture_data.occlusion_image_width,
			texture_data.occlusion_image_height,
			texture_data.occlusion_image_pixels,
			texture_data.occlusion_image_format);

		rhi->CreateGlobalImage(
			texture_data.now_material->emissive_texture_image,
			texture_data.now_material->emissive_image_view,
			texture_data.now_material->emissive_image_allocation,
			texture_data.emissive_image_width,
			texture_data.emissive_image_height,
			texture_data.emissive_image_pixels,
			texture_data.emissive_image_format);
	}

	StaticMeshData RenderSource::loadStaticMesh(std::string mesh_file, AxisAlignedBox& bounding_box)
	{
		StaticMeshData mesh_data;

		//tinyobj::ObjReader       reader;
		//tinyobj::ObjReaderConfig reader_config;
		//reader_config.vertex_color = false;
		//if (!reader.ParseFromFile(filename, reader_config))
		//{
		//	if (!reader.Error().empty())
		//	{
		//		AS_CORE_ERROR("loadMesh {} failed, error: {}", filename, reader.Error());
		//	}
		//	assert(0);
		//}

		//if (!reader.Warning().empty())
		//{
		//	AS_CORE_WARN("loadMesh {} warning, warning: {}", filename, reader.Warning());
		//}

		//auto& attrib = reader.GetAttrib();
		//auto& shapes = reader.GetShapes();

		//std::vector<MeshVertexDataDefinition> mesh_vertices;

		//for (size_t s = 0; s < shapes.size(); s++)
		//{
		//	size_t index_offset = 0;
		//	for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		//	{
		//		size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

		//		bool with_normal = true;
		//		bool with_texcoord = true;

		//		glm::vec3 vertex[3];
		//		glm::vec3 normal[3];
		//		glm::vec2 uv[3];

		//		// only deals with triangle faces
		//		if (fv != 3)
		//		{
		//			continue;
		//		}

		//		// expanding vertex data is not efficient and is for testing purposes only
		//		for (size_t v = 0; v < fv; v++)
		//		{
		//			auto idx = shapes[s].mesh.indices[index_offset + v];
		//			auto vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
		//			auto vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
		//			auto vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

		//			vertex[v].x = static_cast<float>(vx);
		//			vertex[v].y = static_cast<float>(vy);
		//			vertex[v].z = static_cast<float>(vz);

		//			bounding_box.Merge(glm::vec3(vertex[v].x, vertex[v].y, vertex[v].z));

		//			if (idx.normal_index >= 0)
		//			{
		//				auto nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
		//				auto ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
		//				auto nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

		//				normal[v].x = static_cast<float>(nx);
		//				normal[v].y = static_cast<float>(ny);
		//				normal[v].z = static_cast<float>(nz);
		//			}
		//			else
		//			{
		//				with_normal = false;
		//			}

		//			if (idx.texcoord_index >= 0)
		//			{
		//				auto tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
		//				auto ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

		//				uv[v].x = static_cast<float>(tx);
		//				uv[v].y = static_cast<float>(ty);
		//			}
		//			else
		//			{
		//				with_texcoord = false;
		//			}
		//		}
		//		index_offset += fv;

		//		if (!with_normal)
		//		{
		//			glm::vec3 v0 = vertex[1] - vertex[0];
		//			glm::vec3 v1 = vertex[2] - vertex[1];
		//			normal[0] = glm::normalize(glm::cross(v0, v1));
		//			normal[1] = normal[0];
		//			normal[2] = normal[0];
		//		}

		//		if (!with_texcoord)
		//		{
		//			uv[0] = glm::vec2(0.5f, 0.5f);
		//			uv[1] = glm::vec2(0.5f, 0.5f);
		//			uv[2] = glm::vec2(0.5f, 0.5f);
		//		}

		//		glm::vec3 tangent{ 1, 0, 0 };
		//		{
		//			glm::vec3 edge1 = vertex[1] - vertex[0];
		//			glm::vec3 edge2 = vertex[2] - vertex[1];
		//			glm::vec2 deltaUV1 = uv[1] - uv[0];
		//			glm::vec2 deltaUV2 = uv[2] - uv[1];

		//			auto divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
		//			if (divide >= 0.0f && divide < 0.000001f)
		//				divide = 0.000001f;
		//			else if (divide < 0.0f && divide > -0.000001f)
		//				divide = -0.000001f;

		//			float df = 1.0f / divide;
		//			tangent.x = df * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		//			tangent.y = df * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		//			tangent.z = df * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		//			tangent = glm::normalize(tangent);
		//		}

		//		for (size_t i = 0; i < 3; i++)
		//		{
		//			MeshVertexDataDefinition mesh_vert{};

		//			mesh_vert.x = vertex[i].x;
		//			mesh_vert.y = vertex[i].y;
		//			mesh_vert.z = vertex[i].z;

		//			mesh_vert.nx = normal[i].x;
		//			mesh_vert.ny = normal[i].y;
		//			mesh_vert.nz = normal[i].z;

		//			mesh_vert.u = uv[i].x;
		//			mesh_vert.v = uv[i].y;

		//			mesh_vert.tx = tangent.x;
		//			mesh_vert.ty = tangent.y;
		//			mesh_vert.tz = tangent.z;

		//			mesh_vertices.push_back(mesh_vert);
		//		}
		//	}
		//}

		//uint32_t stride = sizeof(MeshVertexDataDefinition);
		//mesh_data.m_vertex_buffer = std::make_shared<BufferData>(mesh_vertices.size() * stride);
		//mesh_data.m_index_buffer = std::make_shared<BufferData>(mesh_vertices.size() * sizeof(uint16_t));

		//assert(mesh_vertices.size() <= std::numeric_limits<uint16_t>::max()); // take care of the index range, should be
		//																	  // consistent with the index range used by
		//																	  // vulkan

		//uint16_t* indices = (uint16_t*)mesh_data.m_index_buffer->m_data;
		//for (size_t i = 0; i < mesh_vertices.size(); i++)
		//{
		//	((MeshVertexDataDefinition*)(mesh_data.m_vertex_buffer->m_data))[i] = mesh_vertices[i];
		//	indices[i] = static_cast<uint16_t>(i);
		//}
		return mesh_data;
	}
}