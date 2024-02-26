#include "aspch.h"
#include "VulkanShader.h"

namespace Astan
{
	VulkanShader::VulkanShader(const std::string& filepath)
	{
	}
	VulkanShader::VulkanShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
	}
	VulkanShader::~VulkanShader()
	{
	}
	void VulkanShader::Bind() const
	{
	}
	void VulkanShader::Unbind() const
	{
	}
	void VulkanShader::SetInt(const std::string& name, int value)
	{
	}
	void VulkanShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
	}
	void VulkanShader::SetFloat(const std::string& name, float value)
	{
	}
	void VulkanShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
	}
	void VulkanShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
	}
	void VulkanShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
	}
	void VulkanShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
	}
	void VulkanShader::UploadUniformInt(const std::string& name, int value)
	{
	}
	void VulkanShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
	}
	void VulkanShader::UploadUniformFloat(const std::string& name, float value)
	{
	}
	void VulkanShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
	}
	void VulkanShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
	}
	void VulkanShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
	}
	void VulkanShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
	}
	void VulkanShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
	}
	std::string VulkanShader::ReadFile(const std::string& filepath)
	{
		return std::string();
	}
	std::unordered_map<GLenum, std::string> VulkanShader::PreProcess(const std::string& source)
	{
		return std::unordered_map<GLenum, std::string>();
	}
	void VulkanShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
	}
	void VulkanShader::CompileOrGetVulkanBinaries()
	{
	}
	void VulkanShader::CreateProgram()
	{
	}
	void VulkanShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
	}
}