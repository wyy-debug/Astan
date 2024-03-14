#include "aspch.h"
#include "RenderSystem.h"
#include <Astan/Core/AssetManager.h>
namespace Astan
{
	RenderSystem::~RenderSystem()
	{
	}

	// 序列化后处理
	void RenderSystem::Initialize()
	{
		// 初始化 RHI
		m_RenderCommand = CreateRef<VulkanRendererAPI>();
		m_RenderCommand->Initialize();
		
		// 设置摄像头
		m_RenderCamera = CreateRef<EditorCamera>();

		// 设置渲染场景
		m_RenderScene = CreateRef<Scene>();

        // 设置渲染资源
        m_RenderResource = CreateRef<RenderSource>();

		// 初始化渲染管线
		m_RenderPipeline = CreateRef<RenderPipeline>();
		m_RenderPipeline->m_RenderCommand = m_RenderCommand;
		m_RenderPipeline->Initialize();

	}


	void RenderSystem::Tick()
	{
		// 资源更新
		ProcessSwapData();

		m_RenderCommand->PrepareContext();

        Scene* scene = std::addressof(*m_RenderScene);

        // 移到scene中处理
        m_RenderResource->UpdatePerFrameBuffer(scene, m_RenderCamera);

        // 在scene场景中处理
        m_RenderResource->UpdateVisibleObjects(scene, m_RenderCamera);

        // prepare pipeline's render passes data
        // 准备管线渲染passes数据
        m_RenderPipeline->PreparePassData(m_RenderResource);

        //g_runtime_global_context.m_debugdraw_manager->tick(delta_time);

        // render one frame
        // 渲染一帧

        m_RenderPipeline->ForwardRender(m_RenderCommand, m_RenderResource);
#if 0
        if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::FORWARD_PIPELINE)
        {
            m_RenderPipeline->ForwardRender(m_RenderCommand, m_render_resource);
        }
        else if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE)
        {
            m_RenderPipeline->DeferredRender(m_RenderCommand, m_render_resource);
        }
        else
        {
            AS_CORE_ERROR(__FUNCTION__, "unsupported render pipeline type");
        }

#endif
	}

    /***
    1. 更新全局资源：如果有全局资源需要更新（例如，场景中的光照或纹理等），它会上传这些资源到渲染硬件接口（如Vulkan）。
    2. 更新游戏对象资源：对于游戏中的每个对象（如角色、道具等），如果它们的资源描述发生变化，这个函数会处理这些变化，包括加载新的网格数据、材质数据，并将它们上传到渲染硬件接口。
    3. 删除游戏对象：如果有游戏对象被标记为删除，该函数会从渲染场景中移除这些对象。
    4. 处理摄像机数据交换：如果有关摄像机的数据（如视角、视图矩阵等）需要更新，该函数会更新渲染系统中的摄像机对象。
    5. 处理粒子系统请求：如果有粒子系统的更新请求，如创建新的粒子发射器、更新粒子发射器的状态等，该函数会处理这些请求。
    6. 处理其他渲染数据交换：此外，该函数还负责处理其他类型的数据交换，如更新渲染场景中的可见对象、准备渲染管线的数据等。
    移到scene中处理
    ***/
    // 全局管理？ Entity
	void RenderSystem::ProcessSwapData()
	{
        struct GameObjectMeshDesc
        {
            std::string m_mesh_file;
        };

        struct GameObjectMaterialDesc
        {
            std::string m_base_color_texture_file;
            std::string m_metallic_roughness_texture_file;
            std::string m_normal_texture_file;
            std::string m_occlusion_texture_file;
            std::string m_emissive_texture_file;
            bool        m_with_texture{ false };
        }

        struct Animation
        {
            bool m_with_animation;
        };

        SkeletonBindingDesc
        std::string m_skeleton_binding_file;

        SkeletonAnimationResult
        Matrix4x4 m_matrix;


        Gameobject
        {
             GameObjectMeshDesc      m_mesh_desc;
        GameObjectMaterialDesc  m_material_desc;
        GameObjectTransformDesc m_transform_desc;
        bool                    m_with_animation {false};
        SkeletonBindingDesc     m_skeleton_binding_desc;
        SkeletonAnimationResult m_skeleton_animation_result;
        }




        return m_swap_data[m_render_swap_data_index]
        struct LevelResourceDesc
        {
            LevelIBLResourceDesc          m_ibl_resource_desc;
            LevelColorGradingResourceDesc m_color_grading_resource_desc;
        };

        struct RenderSwapData
        {
            std::optional<LevelResourceDesc>       m_level_resource_desc;
            std::optional<GameObjectResourceDesc>  m_game_object_resource_desc;
            std::optional<GameObjectResourceDesc>  m_game_object_to_delete;
            std::optional<CameraSwapData>          m_camera_swap_data;
            std::optional<ParticleSubmitRequest>   m_particle_submit_request;
            std::optional<EmitterTickRequest>      m_emitter_tick_request;
            std::optional<EmitterTransformRequest> m_emitter_transform_request;

            void addDirtyGameObject(GameObjectDesc&& desc);
            void addDeleteGameObject(GameObjectDesc&& desc);

            void addNewParticleEmitter(ParticleEmitterDesc& desc);
            void addTickParticleEmitter(ParticleEmitterID id);
            void updateParticleTransform(ParticleEmitterTransformDesc& desc);
        };
        RenderSwapData& swap_data = m_SwapContext.getRenderSwapData();

        Ref<AssetManager> asset_manager = CreateRef<AssetManager>();
        AS_CORE_ASSERT(asset_manager);

        // TODO: update global resources if needed
        // TOD0: 更新全局资源
        if (swap_data.m_level_resource_desc.has_value())
        {
            m_RenderResource->UploadGlobalRenderResource(m_RenderCommand, *swap_data.m_level_resource_desc);

            // reset level resource swap data to a clean state
            m_SwapContext.resetLevelRsourceSwapData();
        }

        // update game object if needed
        // 如果需要的话更新go
        if (swap_data.m_game_object_resource_desc.has_value())
        {
            while (!swap_data.m_game_object_resource_desc->isEmpty())
            {
                GameObjectDesc gobject = swap_data.m_game_object_resource_desc->getNextProcessObject();

                for (size_t part_index = 0; part_index < gobject.getObjectParts().size(); part_index++)
                {
                    const auto& game_object_part = gobject.getObjectParts()[part_index];
                    GameObjectPartId part_id = { gobject.getId(), part_index };

                    bool is_entity_in_scene = m_RenderScene->getInstanceIdAllocator().hasElement(part_id);

                    RenderEntity render_entity;
                    render_entity.m_instance_id =
                        static_cast<uint32_t>(m_RenderScene->getInstanceIdAllocator().allocGuid(part_id));
                    render_entity.m_model_matrix = game_object_part.m_transform_desc.m_transform_matrix;

                    m_RenderScene->addInstanceIdToMap(render_entity.m_instance_id, gobject.getId());

                    // mesh properties
                    MeshSourceDesc mesh_source = { game_object_part.m_mesh_desc.m_mesh_file };
                    bool           is_mesh_loaded = m_RenderScene->getMeshAssetIdAllocator().hasElement(mesh_source);

                    RenderMeshData mesh_data;
                    if (!is_mesh_loaded)
                    {
                        mesh_data = m_RenderScene->loadMeshData(mesh_source, render_entity.m_bounding_box);
                    }
                    else
                    {
                        render_entity.m_bounding_box = m_RenderScene->getCachedBoudingBox(mesh_source);
                    }

                    render_entity.m_mesh_asset_id = m_RenderScene->getMeshAssetIdAllocator().allocGuid(mesh_source);
                    render_entity.m_enable_vertex_blending =
                        game_object_part.m_skeleton_animation_result.m_transforms.size() > 1; // take care
                    render_entity.m_joint_matrices.resize(
                        game_object_part.m_skeleton_animation_result.m_transforms.size());
                    for (size_t i = 0; i < game_object_part.m_skeleton_animation_result.m_transforms.size(); ++i)
                    {
                        render_entity.m_joint_matrices[i] =
                            game_object_part.m_skeleton_animation_result.m_transforms[i].m_matrix;
                    }

                    // material properties
                    // 材质属性
                    MaterialSourceDesc material_source;
                    if (game_object_part.m_material_desc.m_with_texture)
                    {
                        material_source = { game_object_part.m_material_desc.m_base_color_texture_file,
                                           game_object_part.m_material_desc.m_metallic_roughness_texture_file,
                                           game_object_part.m_material_desc.m_normal_texture_file,
                                           game_object_part.m_material_desc.m_occlusion_texture_file,
                                           game_object_part.m_material_desc.m_emissive_texture_file };
                    }
                    else
                    {
                        // TODO: move to default material definition json file
                        material_source = {
                            asset_manager->getFullPath("asset/texture/default/albedo.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/mr.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/normal.jpg").generic_string(),
                            "",
                            "" };
                    }
                    bool is_material_loaded = m_RenderScene->getMaterialAssetdAllocator().hasElement(material_source);

                    RenderMaterialData material_data;
                    if (!is_material_loaded)
                    {
                        material_data = m_RenderScene->loadMaterialData(material_source);
                    }

                    render_entity.m_material_asset_id =
                        m_RenderScene->getMaterialAssetdAllocator().allocGuid(material_source);

                    // create game object on the graphics api side
                    if (!is_mesh_loaded)
                    {
                        m_RenderScene->uploadGameObjectRenderResource(m_RenderCommand, render_entity, mesh_data);
                    }

                    if (!is_material_loaded)
                    {
                        m_RenderScene->uploadGameObjectRenderResource(m_RenderCommand, render_entity, material_data);
                    }

                    // add object to render scene if needed
                    if (!is_entity_in_scene)
                    {
                        m_RenderScene->m_render_entities.push_back(render_entity);
                    }
                    else
                    {
                        for (auto& entity : m_RenderScene->m_render_entities)
                        {
                            if (entity.m_instance_id == render_entity.m_instance_id)
                            {
                                entity = render_entity;
                                break;
                            }
                        }
                    }
                }
                // after finished processing, pop this game object
                swap_data.m_game_object_resource_desc->pop();
            }

            // reset game object swap data to a clean state
            m_SwapContext.resetGameObjectResourceSwapData();
        }

        // remove deleted objects
        if (swap_data.m_game_object_to_delete.has_value())
        {
            while (!swap_data.m_game_object_to_delete->isEmpty())
            {
                GameObjectDesc gobject = swap_data.m_game_object_to_delete->getNextProcessObject();
                m_RenderScene->deleteEntityByGObjectID(gobject.getId());
                swap_data.m_game_object_to_delete->pop();
            }

            m_SwapContext.resetGameObjectToDelete();
        }

        // process camera swap data
        if (swap_data.m_camera_swap_data.has_value())
        {
            if (swap_data.m_camera_swap_data->m_fov_x.has_value())
            {
                m_render_camera->setFOVx(*swap_data.m_camera_swap_data->m_fov_x);
            }

            if (swap_data.m_camera_swap_data->m_view_matrix.has_value())
            {
                m_render_camera->setMainViewMatrix(*swap_data.m_camera_swap_data->m_view_matrix);
            }

            if (swap_data.m_camera_swap_data->m_camera_type.has_value())
            {
                m_render_camera->setCurrentCameraType(*swap_data.m_camera_swap_data->m_camera_type);
            }

            m_SwapContext.resetCameraSwapData();
        }
#if 0
        if (swap_data.m_particle_submit_request.has_value())
        {
            std::shared_ptr<ParticlePass> particle_pass =
                std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass);

            int emitter_count = swap_data.m_particle_submit_request->getEmitterCount();
            particle_pass->setEmitterCount(emitter_count);

            for (int index = 0; index < emitter_count; ++index)
            {
                const ParticleEmitterDesc& desc = swap_data.m_particle_submit_request->getEmitterDesc(index);
                particle_pass->createEmitter(index, desc);
            }

            particle_pass->initializeEmitters();

            m_SwapContext.resetPartilceBatchSwapData();
        }
        if (swap_data.m_emitter_tick_request.has_value())
        {
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
                ->setTickIndices(swap_data.m_emitter_tick_request->m_emitter_indices);
            m_SwapContext.resetEmitterTickSwapData();
        }

        if (swap_data.m_emitter_transform_request.has_value())
        {
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
                ->setTransformIndices(swap_data.m_emitter_transform_request->m_transform_descs);
            m_SwapContext.resetEmitterTransformSwapData();
        }
#endif
	}
}