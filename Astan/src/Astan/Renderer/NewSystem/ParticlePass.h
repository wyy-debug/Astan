#pragma once
#include "RenderPass.h"
#include <Astan/Scene/Scene.h>
#include "RenderSource.h"
#include <glm/glm.hpp>
#include <Astan/Particle/ParticleDesc.h>
#include <Astan/Particle/ParticleManager.h>

#include <algorithm>
#include <cfloat>
#include <random>
namespace Astan
{
    struct ParticlePassInitInfo : RenderPass::RenderPassInitInfo
    {
        std::shared_ptr<ParticleManager> m_particle_manager;
    };

    template<typename RandomEngine = std::default_random_engine>
    class RandomNumberGenerator
    {

    private:
        RandomEngine m_engine;

    public:
        template<typename... Params>
        explicit RandomNumberGenerator(Params&&... params) : m_engine(std::forward<Params>(params)...)
        {}

        template<typename... Params>
        void seed(Params&&... seeding)
        {
            m_engine.seed(std::forward<Params>(seeding)...);
        }

        template<typename DistributionFunc, typename... Params>
        typename DistributionFunc::result_type distribution(Params&&... params)
        {
            DistributionFunc dist(std::forward<Params>(params)...);
            return dist(m_engine);
        }

        float uniformUnit() { return uniformDistribution(0.f, std::nextafter(1.f, FLT_MAX)); }

        float uniformSymmetry() { return uniformDistribution(-1.f, std::nextafter(1.f, FLT_MAX)); }

        bool bernoulliDistribution(float probability) { return distribution<std::bernoulli_distribution>(probability); }

        float normalDistribution(float mean, float stddev)
        {
            return distribution<std::normal_distribution<float>>(mean, stddev);
        }

        template<typename DistributionFunc, typename Range, typename... Params>
        void generator(Range&& range, Params&&... params)
        {
            // using ResultType = typename DistributionFunc::result_type;

            DistributionFunc dist(std::forward<Params>(params)...);
            return std::generate(std::begin(range), std::end(range), [&] { return dist(m_engine); });
        }
    };

    using DefaultRNG = RandomNumberGenerator<std::mt19937>;

    class ParticleEmitterBufferBatch
    {
    public:
        RHIBuffer* m_position_render_buffer = nullptr;
        RHIBuffer* m_position_device_buffer = nullptr;
        RHIBuffer* m_position_host_buffer = nullptr;
        RHIBuffer* m_counter_device_buffer = nullptr;
        RHIBuffer* m_counter_host_buffer = nullptr;
        RHIBuffer* m_indirect_dispatch_argument_buffer = nullptr;
        RHIBuffer* m_alive_list_buffer = nullptr;
        RHIBuffer* m_alive_list_next_buffer = nullptr;
        RHIBuffer* m_dead_list_buffer = nullptr;
        RHIBuffer* m_particle_component_res_buffer = nullptr;

        RHIDeviceMemory* m_counter_host_memory = nullptr;
        RHIDeviceMemory* m_position_host_memory = nullptr;
        RHIDeviceMemory* m_position_device_memory = nullptr;
        RHIDeviceMemory* m_counter_device_memory = nullptr;
        RHIDeviceMemory* m_indirect_dispatch_argument_memory = nullptr;
        RHIDeviceMemory* m_alive_list_memory = nullptr;
        RHIDeviceMemory* m_alive_list_next_memory = nullptr;
        RHIDeviceMemory* m_dead_list_memory = nullptr;
        RHIDeviceMemory* m_particle_component_res_memory = nullptr;
        RHIDeviceMemory* m_position_render_memory = nullptr;

        void* m_emitter_desc_mapped{ nullptr };

        ParticleEmitterDesc m_emitter_desc;

        uint32_t m_num_particle{ 0 };
        void     freeUpBatch(std::shared_ptr<VulkanRendererAPI> rhi);
    };

    class ParticlePass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* init_info) override final;

        void PreparePassData(Ref<RenderSource> source) ;

        void Draw() override final;

        void Simulate();

        void CopyNormalAndDepthImage();

        void SetDepthAndNormalImage(RHIImage* depth_image, RHIImage* normal_image);

        void SetupParticlePass();

        void SetRenderCommandBufferHandle(RHICommandBuffer* command_buffer);

        void SetRenderPassHandle(RHIRenderPass* render_pass);

        void UpdateAfterFramebufferRecreate();

        void SetEmitterCount(int count);

        void CreateEmitter(int id, const ParticleEmitterDesc& desc);

        void InitializeEmitters();

        void SetTickIndices(const std::vector<ParticleEmitterID>& tick_indices);

        void SetTransformIndices(const std::vector<ParticleEmitterTransformDesc>& transform_indices);

    private:
        void UpdateUniformBuffer();

        void UpdateEmitterTransform();

        void SetupAttachments();

        void SetupDescriptorSetLayout();

        void PrepareUniformBuffer();

        void SetupPipelines();

        void AllocateDescriptorSet();

        void UpdateDescriptorSet();

        void SetupParticleDescriptorSet();

        RHIPipeline* m_kickoff_pipeline = nullptr;
        RHIPipeline* m_emit_pipeline = nullptr;
        RHIPipeline* m_simulate_pipeline = nullptr;

        RHICommandBuffer* m_compute_command_buffer = nullptr;
        RHICommandBuffer* m_render_command_buffer = nullptr;
        RHICommandBuffer* m_copy_command_buffer = nullptr;

        RHIBuffer* m_scene_uniform_buffer = nullptr;
        RHIBuffer* m_compute_uniform_buffer = nullptr;
        RHIBuffer* m_particle_billboard_uniform_buffer = nullptr;

        RHIViewport m_viewport_params;

        RHIFence* m_fence = nullptr;

        RHIImage* m_src_depth_image = nullptr;
        RHIImage* m_dst_normal_image = nullptr;
        RHIImage* m_src_normal_image = nullptr;
        RHIImage* m_dst_depth_image = nullptr;
        RHIImageView* m_src_depth_image_view = nullptr;
        RHIImageView* m_src_normal_image_view = nullptr;
        RHIDeviceMemory* m_dst_normal_image_memory = nullptr;
        RHIDeviceMemory* m_dst_depth_image_memory = nullptr;

        /*
         * particle rendering
         */
        RHIImage* m_particle_billboard_texture_image = nullptr;
        RHIImageView* m_particle_billboard_texture_image_view = nullptr;
        VmaAllocation m_particle_billboard_texture_vma_allocation;

        RHIImage* m_piccolo_logo_texture_image = nullptr;
        RHIImageView* m_piccolo_logo_texture_image_view = nullptr;
        VmaAllocation m_piccolo_logo_texture_vma_allocation;

        RHIRenderPass* m_render_pass;

        ParticleBillboardPerframeStorageBufferObject m_particlebillboard_perframe_storage_buffer_object;
        ParticleCollisionPerframeStorageBufferObject m_particle_collision_perframe_storage_buffer_object;

        void* m_particle_compute_buffer_mapped{ nullptr };
        void* m_particle_billboard_uniform_buffer_mapped{ nullptr };
        void* m_scene_uniform_buffer_mapped{ nullptr };

        struct uvec4
        {
            uint32_t x;
            uint32_t y;
            uint32_t z;
            uint32_t w;
        };

        struct computeUniformBufferObject
        {
            int     emit_gap;
            int     xemit_count;
            float   max_life;
            float   time_step;
            glm::vec4 pack; // randomness 3 | frame index 1
            glm::vec3 gravity;
            float   padding;
            uvec4   viewport; // x, y, width, height
            glm::vec4 extent;   // width, height, near, far
        } m_ubo;

        struct Particle
        {
            glm::vec3 pos;
            float   life;
            glm::vec3 vel;
            float   size_x;
            glm::vec3 acc;
            float   size_y;
            glm::vec4 color;
        };

        // indirect dispath parameter offset
        static const uint32_t s_argument_offset_emit = 0;
        static const uint32_t s_argument_offset_simulate = s_argument_offset_emit + sizeof(uvec4);
        struct IndirectArgumemt
        {
            uvec4 emit_argument;
            uvec4 simulate_argument;
            int   alive_flap_bit;
        };

        struct ParticleCounter
        {
            int dead_count;
            int alive_count;
            int alive_count_after_sim;
            int emit_count;
        };

        std::vector<ParticleEmitterBufferBatch> m_emitter_buffer_batches;
        std::shared_ptr<ParticleManager>        m_particle_manager;

        DefaultRNG m_random_engine;

        int m_emitter_count;

        static constexpr bool s_verbose_particle_alive_info{ false };

        std::vector<ParticleEmitterID> m_emitter_tick_indices;

        std::vector<ParticleEmitterTransformDesc> m_emitter_transform_indices;
    };
} // namespace Piccolo
