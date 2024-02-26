#include "aspch.h"
#include "VulkanFramebuffer.h"

namespace Astan
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
	{
	}
	VulkanFramebuffer::~VulkanFramebuffer()
	{
	}
	void VulkanFramebuffer::Invalidate()
	{
	}
	void VulkanFramebuffer::Bind()
	{
	}
	void VulkanFramebuffer::Unbind()
	{
	}
	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
	}
	int VulkanFramebuffer::ReadPixel(uint32_t attachemntIndex, int x, int y)
	{
		return 0;
	}
	void VulkanFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
	}
}