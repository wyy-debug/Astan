#include "aspch.h"
#include "RenderSystem.h"
#include "Astan/Scene/Entity.h"
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

        Ref<AssetManager> asset_manager = CreateRef<AssetManager>();
        AS_CORE_ASSERT(asset_manager);

        // TOD0
        //if (swap_data.m_level_resource_desc.has_value())
        //{
        //    m_RenderResource->UploadGlobalRenderResource(m_RenderCommand, *swap_data.m_level_resource_desc);

        //    // reset level resource swap data to a clean state
        //    m_SwapContext.resetLevelRsourceSwapData();
        //}

        // update game object if needed
        // 如果需要的话更新go
        
        auto view = m_RenderScene->GetAllEntitiesWith<RenderEntityComponent>();

        for (auto e : view)
        {
            Scene* scene = std::addressof(*m_RenderScene);
            Entity entity = { e,scene };
            // Mesh
            {
                auto renderComponent = entity.GetComponent<RenderEntityComponent>();
                // not load mesh
                RenderMeshData meshData;
                auto gameObjectComponent = entity.GetComponent<GameObjectMeshComponent>();
                meshData = m_RenderResource->LoadMeshData(gameObjectComponent.m_mesh_file, renderComponent.m_BoundingBox);
                // load mesh
                renderComponent.m_BoundingBox = m_RenderResource->GetCachedBoudingBox(gameObjectComponent.m_mesh_file);

                auto skeletonAnimationComponent = entity.GetComponent<SkeletonAnimationResultComponent>();
                renderComponent.m_EnableVertexBlending = skeletonAnimationComponent.m_transforms.size() > 1;
                renderComponent.m_JointMatrices.resize(skeletonAnimationComponent.m_transforms.size());
                
                for (size_t i = 0; i < skeletonAnimationComponent.m_transforms.size(); i++)
                {
                    renderComponent.m_JointMatrices[i] = skeletonAnimationComponent.m_transforms[i];
                }
            }
            // Material

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