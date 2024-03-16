#pragma once

#include <array>
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Astan
{
	struct MeshVertex
	{
		struct VulkanMeshVertexPosition
		{
			glm::vec3 position;
		};

		struct VulkanMeshVertexVaringEnableBlending
		{
			glm::vec3 normal;
			glm::vec3 tangent;
		};

		struct VulkanMeshVertexVaring
		{
			glm::vec2 texcoord;
		};

		struct VulkanMeshVertexJointBinding
		{
			glm::ivec4 indices;
			glm::vec4 weights;
		};

		static std::array<RHIVertexInputBindingDescription, 3> GetBindingDescriptions()
		{
			std::array<RHIVertexInputBindingDescription, 3> bindingDescriptions{};
			
			// position
			bindingDescriptions[0].binding = 0;
			bindingDescriptions[0].stride = sizeof(VulkanMeshVertexPosition);
			bindingDescriptions[0].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;

			// varying blending
			bindingDescriptions[1].binding = 1;
			bindingDescriptions[1].stride = sizeof(VulkanMeshVertexVaringEnableBlending);
			bindingDescriptions[1].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;

			// varying
			bindingDescriptions[2].binding = 2;
			bindingDescriptions[2].stride = sizeof(VulkanMeshVertexVaring);
			bindingDescriptions[2].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescriptions;
		}

		static std::array<RHIVertexInputAttributeDescription, 4> GetAttributeDescriptions()
		{
			std::array<RHIVertexInputAttributeDescription, 4> attributeDescriptions;

			// position
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = RHI_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(VulkanMeshVertexPosition, position);

			// varying blending
			attributeDescriptions[0].binding = 1;
			attributeDescriptions[0].location = 1;
			attributeDescriptions[0].format = RHI_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(VulkanMeshVertexVaringEnableBlending, normal);
			attributeDescriptions[1].binding = 1;
			attributeDescriptions[1].location = 2;
			attributeDescriptions[1].format = RHI_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(VulkanMeshVertexVaringEnableBlending, tangent);

			// varying
			attributeDescriptions[2].binding = 2;
			attributeDescriptions[2].location = 3;
			attributeDescriptions[2].format = RHI_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(VulkanMeshVertexVaring, texcoord);
			
			return attributeDescriptions;
		}
	};
}