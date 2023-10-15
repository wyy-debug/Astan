#include "Sandbox2D.h"
#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{

}


void Sandbox2D::OnAttach()
{

	m_SquareVA = Astan::VertexArray::Create();
	float squareVertices[5 * 4] = {
		-0.5f,-0.5f,0.0f,
		 0.5f,-0.5f,0.0f,
		 0.5f, 0.5f,0.0f,
		-0.5f, 0.5f,0.0f
	};

	Astan::Ref<Astan::VertexBuffer> squreVB;
	squreVB.reset(Astan::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
	squreVB->SetLayout({
		{Astan::ShaderDataType::Float3, "a_Position"},
		});
	m_SquareVA->AddVertexBuffer(squreVB);

	uint32_t squareIndices[6] = { 0,1,2,2,3,0 };
	Astan::Ref<Astan::IndexBuffer> squareIB;
	squareIB.reset(Astan::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
	m_SquareVA->SetIndexBuffer(squareIB);

	m_FlatColorShader = Astan::Shader::Create("assets/shaders/FlatColor.glsl");

}

void Sandbox2D::OnDetach()
{}


void Sandbox2D::OnUpdate(Astan::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	Astan::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Astan::RenderCommand::Clear();

	Astan::Renderer::BeginScene(m_CameraController.GetCamera());

	std::dynamic_pointer_cast<Astan::OpenGLShader>(m_FlatColorShader)->Bind();
	std::dynamic_pointer_cast<Astan::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);

	//m_FlatColorShader->Bind();
	Astan::Renderer::Submit(m_FlatColorShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

	Astan::Renderer::EndScene();

}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Setting");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Astan::Event& event)
{
	m_CameraController.OnEvent(event);
}