#include "aspch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"

#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Astan
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColorShader;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init() 
	{
		s_Data = new Renderer2DStorage();
		s_Data->QuadVertexArray = VertexArray::Create();
		float squareVertices[5 * 4] = {
			-0.5f,-0.5f,0.0f,
			 0.5f,-0.5f,0.0f,
			 0.5f, 0.5f,0.0f,
			-0.5f, 0.5f,0.0f
		};

		Ref<VertexBuffer> squreVB;
		squreVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squreVB->SetLayout({
			{ShaderDataType::Float3, "a_Position"},
			});
		s_Data->QuadVertexArray->AddVertexBuffer(squreVB);

		uint32_t squareIndices[6] = { 0,1,2,2,3,0 };
		Ref<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

		s_Data->FlatColorShader = Shader::Create("assets/shaders/FlatColor.glsl");
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_ViewProjection",camera.GetProjectionMatrix());
	}

	void Renderer2D::EndScene() 
	{
	}

	void Renderer2D::DrawQuad(const glm::vec2& positon, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ positon.x,positon.y,0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& positon, const glm::vec2& size, const glm::vec4& color)
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Astan::OpenGLShader>(s_Data->FlatColorShader)->UploadUniformFloat4("u_Color", color);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}
}