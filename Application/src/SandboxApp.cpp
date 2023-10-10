#include <Astan.h>

#include "imgui/imgui.h"


class ExampleLayer : public Astan::Layer
{
public:
	ExampleLayer()
		:Layer("Exampler"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f,0.0f,0.0f)
	{
		m_VertexArray.reset(Astan::VertexArray::Create());


		float vertices[3 * 7] = {
			-0.5f,-0.5f,0.0f, 0.8f,0.2f,0.8f,1.0f,
			 0.5f,-0.5f,0.0f, 0.2f,0.3f,0.8f,1.0f,
			 0.0f, 0.5f,0.0f, 0.8f,0.8f,0.2f,1.0f,
		};
		std::shared_ptr<Astan::VertexBuffer> vertexBuffer;

		vertexBuffer.reset(Astan::VertexBuffer::Create(vertices, sizeof(vertices)));

		Astan::BufferLayout layout =
		{
			{Astan::ShaderDataType::Float3, "a_Position"},
			{Astan::ShaderDataType::Float4, "a_Color"}
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);


		uint32_t indices[3] = { 0,1,2 };
		std::shared_ptr<Astan::IndexBuffer> indexBuffer;
		indexBuffer.reset(Astan::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);


		m_SquareVA.reset(Astan::VertexArray::Create());
		float squareVertices[3 * 4] = {
			-0.75f,-0.75f,0.0f,
			 0.75f,-0.75f,0.0f,
			 0.75f, 0.75f,0.0f,
			-0.75f, 0.75f,0.0f
		};
		std::shared_ptr<Astan::VertexBuffer> squreVB;
		squreVB.reset(Astan::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squreVB->SetLayout({
			{Astan::ShaderDataType::Float3, "a_Position"},
			});
		m_SquareVA->AddVertexBuffer(squreVB);

		uint32_t squareIndices[6] = { 0,1,2,2,3,0 };
		std::shared_ptr<Astan::IndexBuffer> squareIB;
		squareIB.reset(Astan::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string vertexSource = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			
			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position,1.0);
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
		m_Shader.reset(new Astan::Shader(vertexSource, fragmentSource));

		std::string blueShaderVertexSource = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;			

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection *  vec4(a_Position,1.0);
			}
		)";
		std::string blueShaderFragmentSource = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			in vec3 v_Position;

			void main()
			{
				color = vec4(0.2,0.3,0.8, 1.0);
			}
		)";
		m_BlueShader.reset(new Astan::Shader(blueShaderVertexSource, blueShaderFragmentSource));
	}

	virtual void OnImGuiRender() override
	{

	}
	void OnUpdate() override
	{
		if (Astan::Input::IsKeyPressed(AS_KEY_LEFT))
			m_CameraPosition.x += m_CameraMoveSpeed;

		else if (Astan::Input::IsKeyPressed(AS_KEY_RIGHT))
			m_CameraPosition.x -= m_CameraMoveSpeed;

		if (Astan::Input::IsKeyPressed(AS_KEY_DOWN))
			m_CameraPosition.y += m_CameraMoveSpeed;

		else if (Astan::Input::IsKeyPressed(AS_KEY_UP))
			m_CameraPosition.y -= m_CameraMoveSpeed;
		
		if (Astan::Input::IsKeyPressed(AS_KEY_A))
			m_CameraRotation += m_CameraRotationSpeed;

		else if (Astan::Input::IsKeyPressed(AS_KEY_D))
			m_CameraRotation -= m_CameraRotationSpeed;

		Astan::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Astan::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Astan::Renderer::BeginScene(m_Camera);

		Astan::Renderer::Submit(m_BlueShader, m_SquareVA);
		Astan::Renderer::Submit(m_Shader, m_VertexArray);

		Astan::Renderer::EndScene();
	}

	void OnEvent(Astan::Event& event) override
	{

	}




private:
	std::shared_ptr<Astan::Shader> m_Shader;
	std::shared_ptr<Astan::VertexArray> m_VertexArray;


	std::shared_ptr<Astan::VertexArray> m_SquareVA;
	std::shared_ptr<Astan::Shader> m_BlueShader;

	Astan::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 0.1f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 2.0f;
};


class Sandbox : public Astan::Application
{
public:
	Sandbox() 
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() {}
};

Astan::Application* Astan::CreateApplication()
{
	return new Sandbox();
}
