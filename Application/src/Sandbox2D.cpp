#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{

}


void Sandbox2D::OnAttach()
{

}

void Sandbox2D::OnDetach()
{}


void Sandbox2D::OnUpdate(Astan::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	Astan::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Astan::RenderCommand::Clear();

	Astan::Renderer2D::BeginScene(m_CameraController.GetCamera());
	Astan::Renderer2D::DrawQuad({ -1.0f,0.0f }, { 0.8f,0.8f }, { 0.8f,0.2f,0.3f,1.0f });
	Astan::Renderer2D::DrawQuad({ 0.5f,-0.5f }, { 0.5f,0.5f }, { 0.2f,0.3f,0.8f,1.0f });

	Astan::Renderer2D::EndScene();

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