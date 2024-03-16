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
    1. ����ȫ����Դ�������ȫ����Դ��Ҫ���£����磬�����еĹ��ջ������ȣ��������ϴ���Щ��Դ����ȾӲ���ӿڣ���Vulkan����
    2. ������Ϸ������Դ��������Ϸ�е�ÿ���������ɫ�����ߵȣ���������ǵ���Դ���������仯����������ᴦ����Щ�仯�����������µ��������ݡ��������ݣ����������ϴ�����ȾӲ���ӿڡ�
    3. ɾ����Ϸ�����������Ϸ���󱻱��Ϊɾ�����ú��������Ⱦ�������Ƴ���Щ����
    4. ������������ݽ���������й�����������ݣ����ӽǡ���ͼ����ȣ���Ҫ���£��ú����������Ⱦϵͳ�е����������
    5. ��������ϵͳ�������������ϵͳ�ĸ��������紴���µ����ӷ��������������ӷ�������״̬�ȣ��ú����ᴦ����Щ����
    6. ����������Ⱦ���ݽ��������⣬�ú������������������͵����ݽ������������Ⱦ�����еĿɼ�����׼����Ⱦ���ߵ����ݵȡ�
    �Ƶ�scene�д���
    ***/
    // ȫ�ֹ����� Entity
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
            auto renderComponent = entity.GetComponent<RenderEntityComponent>();
            // Mesh
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
            // Material
            
            MaterialSourceDesc materialSourceDesc;
            auto materialComponent = entity.GetComponent<GameObjectMaterialComponent>();
            if(materialComponent.m_with_texture)
            {
                materialSourceDesc =
                {
                    materialComponent.m_base_color_texture_file,
                    materialComponent.m_metallic_roughness_texture_file,
                    materialComponent.m_normal_texture_file,
                    materialComponent.m_occlusion_texture_file,
                    materialComponent.m_emissive_texture_file
                };
            }
            else
            {
                materialSourceDesc = 
                {
                    asset_manager->getFullPath("asset/texture/default/albedo.jpg").generic_string(),
                    asset_manager->getFullPath("asset/texture/default/mr.jpg").generic_string(),
                    asset_manager->getFullPath("asset/texture/default/normal.jpg").generic_string(),
                    "",
                    ""
                };
            }
            // not load
            RenderMaterialData materialData;
            materialData = m_RenderResource->LoadMaterialData(materialSourceDesc);

            // renderComponent.m_MaterialAssetId = 
            m_RenderResource->UploadGameObjectRenderResource(m_RenderCommand,renderComponent,meshData);

            m_RenderResource->UploadGameObjectRenderResource(m_RenderCommand,renderComponent,materialData);

        }

	}
}