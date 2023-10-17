#include "aspch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>
namespace Astan
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Astan::ShaderDataType::Float:    return GL_FLOAT;
			case Astan::ShaderDataType::Float2:   return GL_FLOAT;
			case Astan::ShaderDataType::Float3:   return GL_FLOAT;
			case Astan::ShaderDataType::Float4:   return GL_FLOAT;
			case Astan::ShaderDataType::Mat3:     return GL_FLOAT;
			case Astan::ShaderDataType::Mat4:     return GL_FLOAT;
			case Astan::ShaderDataType::Int:      return GL_INT;
			case Astan::ShaderDataType::Int2:	  return GL_INT;
			case Astan::ShaderDataType::Int3:	  return GL_INT;
			case Astan::ShaderDataType::Int4:	  return GL_INT;
			case Astan::ShaderDataType::Bool:	  return GL_BOOL;
		}
		AS_CORE_ASSERT(false, "Unknown ShaderDatatype!");
		return 0;
	}
	OpenGLVertexArray::OpenGLVertexArray()
	{
		AS_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		AS_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		AS_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		AS_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AS_PROFILE_FUNCTION();

		AS_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layouts!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();


		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}
		m_VertexBuffers.push_back(vertexBuffer);

	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		AS_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffers = indexBuffer;

	}
}