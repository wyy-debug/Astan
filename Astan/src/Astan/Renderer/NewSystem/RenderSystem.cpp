#include "aspch.h"
#include "RenderSystem.h"
#include "Astan/Scene/Entity.h"
#include <Astan/Core/AssetManager.h>
namespace Astan
{
	RenderSystem::~RenderSystem()
	{
	}

	// ���л�����
	void RenderSystem::Initialize()
	{
		// ��ʼ�� RHI
		m_RenderCommand = CreateRef<VulkanRendererAPI>();
		m_RenderCommand->Initialize();
		
		// ��������ͷ
		m_RenderCamera = CreateRef<EditorCamera>();

		// ������Ⱦ����
		m_RenderScene = CreateRef<Scene>();

        // ������Ⱦ��Դ
        m_RenderResource = CreateRef<RenderSource>();

		// ��ʼ����Ⱦ����
		m_RenderPipeline = CreateRef<RenderPipeline>();
		m_RenderPipeline->m_RenderCommand = m_RenderCommand;
		m_RenderPipeline->Initialize();

	}


	void RenderSystem::Tick()
	{
		// ��Դ����
		ProcessSwapData();

		m_RenderCommand->PrepareContext();

        Scene* scene = std::addressof(*m_RenderScene);

        // �Ƶ�scene�д���
        m_RenderResource->UpdatePerFrameBuffer(scene, m_RenderCamera);

        // ��scene�����д���
        m_RenderResource->UpdateVisibleObjects(scene, m_RenderCamera);

        // prepare pipeline's render passes data
        // ׼��������Ⱦpasses����
        m_RenderPipeline->PreparePassData(m_RenderResource);

        //g_runtime_global_context.m_debugdraw_manager->tick(delta_time);

        // render one frame
        // ��Ⱦһ֡

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
    1. ����ȫ����Դ�������ȫ����Դ��Ҫ���£����磬�����еĹ��ջ�����ȣ��������ϴ���Щ��Դ����ȾӲ���ӿڣ���Vulkan����
    2. ������Ϸ������Դ��������Ϸ�е�ÿ���������ɫ�����ߵȣ���������ǵ���Դ���������仯����������ᴦ����Щ�仯�����������µ��������ݡ��������ݣ����������ϴ�����ȾӲ���ӿڡ�
    3. ɾ����Ϸ�����������Ϸ���󱻱��Ϊɾ�����ú��������Ⱦ�������Ƴ���Щ����
    4. ������������ݽ���������й�����������ݣ����ӽǡ���ͼ����ȣ���Ҫ���£��ú����������Ⱦϵͳ�е����������
    5. ��������ϵͳ�������������ϵͳ�ĸ��������紴���µ����ӷ��������������ӷ�������״̬�ȣ��ú����ᴦ����Щ����
    6. ����������Ⱦ���ݽ��������⣬�ú������������������͵����ݽ������������Ⱦ�����еĿɼ�����׼����Ⱦ���ߵ����ݵȡ�
    �Ƶ�scene�д���
    ***/
    // ȫ�ֹ��� Entity
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
        // �����Ҫ�Ļ�����go
        
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