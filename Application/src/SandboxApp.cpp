#include <Astan.h>
#include "Astan/Core/EntryPoint.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "sandbox2D.h"

class ExampleLayer : public Astan::Layer
{
public:
	ExampleLayer()
		:Layer("Exampler"), m_CameraController(1280.0f / 720.0f,true)
	{
		m_VertexArray = Astan::VertexArray::Create();


		float vertices[3 * 7] = {
			-0.5f,-0.5f,0.0f, 0.8f,0.2f,0.8f,1.0f,
			 0.5f,-0.5f,0.0f, 0.2f,0.3f,0.8f,1.0f,
			 0.0f, 0.5f,0.0f, 0.8f,0.8f,0.2f,1.0f,
		};
		Astan::Ref<Astan::VertexBuffer> vertexBuffer;

		vertexBuffer.reset(Astan::VertexBuffer::Create(vertices, sizeof(vertices)));

		Astan::BufferLayout layout =
		{
			{Astan::ShaderDataType::Float3, "a_Position"},
			{Astan::ShaderDataType::Float4, "a_Color"}
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);


		uint32_t indices[3] = { 0,1,2 };
		Astan::Ref<Astan::IndexBuffer> indexBuffer;
		indexBuffer.reset(Astan::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);


		m_SquareVA = Astan::VertexArray::Create();
		float squareVertices[5 * 4] = {
			-0.5f,-0.5f,0.0f,0.0f,0.0f,
			 0.5f,-0.5f,0.0f,1.0f,0.0f,
			 0.5f, 0.5f,0.0f,1.0f,1.0f,
			-0.5f, 0.5f,0.0f,0.0f,1.0f
		};

		Astan::Ref<Astan::VertexBuffer> squreVB;
		squreVB.reset(Astan::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squreVB->SetLayout({
			{Astan::ShaderDataType::Float3, "a_Position"},
			{Astan::ShaderDataType::Float2, "a_TexCoord"},

			});
		m_SquareVA->AddVertexBuffer(squreVB);

		uint32_t squareIndices[6] = { 0,1,2,2,3,0 };
		Astan::Ref<Astan::IndexBuffer> squareIB;
		squareIB.reset(Astan::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string vertexSource = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position,1.0);
			}
		)";
		std::string fragmentSource = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position*0.5+0.5, 1.0);
				color = v_Color;
			}
		)";
		m_Shader = Astan::Shader::Create("vertexPosColor", vertexSource, fragmentSource);


		std::string flatColorShaderVertexSrc = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;			

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position,1.0);
			}
		)";

 
		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			
			uniform vec3 u_Color;
						
			void main()
			{
				color = vec4(u_Color,1.0f);
			}
		)";
		m_FlatColorShader =  Astan::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

		auto textureShader =  m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = Astan::Texture2D::Create("assets/textures/Checkerboard.png");
		m_ChernoLogoTexture = Astan::Texture2D::Create("assets/textures/ChernoLogo.png");



		std::dynamic_pointer_cast<Astan::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Astan::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}


	void OnUpdate(Astan::Timestep ts) override
	{
		m_CameraController.OnUpdate(ts);

		Astan::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Astan::RenderCommand::Clear();

		Astan::Renderer::BeginScene(m_CameraController.GetCamera());



		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		
		
		std::dynamic_pointer_cast<Astan::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Astan::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color",m_SquareColor);

		for (int y = 0; y < 20; y++)
		{
			for (int i = 0; i < 20; i++)
			{
				glm::vec3 pos(i * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Astan::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Astan::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		m_ChernoLogoTexture->Bind();
		Astan::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		//Triangle
		//Astan::Renderer::Submit(m_Shader, m_VertexArray);

		Astan::Renderer::EndScene();
	}


	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Setting");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Astan::Event& event) override
	{
		m_CameraController.OnEvent(event);
	}

private:
	Astan::ShaderLibrary m_ShaderLibrary;
	Astan::Ref<Astan::Shader> m_Shader;
	Astan::Ref<Astan::VertexArray> m_VertexArray;

	
	Astan::Ref<Astan::VertexArray> m_SquareVA;
	Astan::Ref<Astan::Shader> m_FlatColorShader;

	Astan::Ref<Astan::Texture> m_Texture, m_ChernoLogoTexture;

	Astan::OrthographicCameraController m_CameraController;

	glm::vec3 m_SquareColor = { 0.2f,0.3f,0.8f };

};


class Sandbox : public Astan::Application
{
public:
	Sandbox() 
	{
		PushLayer(new Sandbox2D());
	}

	~Sandbox() {}
};

Astan::Application* Astan::CreateApplication()
{
	return new Sandbox();
}
