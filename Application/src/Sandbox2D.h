#pragma once
#include "Astan.h"
#include "ParticleSystem.h"

class Sandbox2D : public Astan::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;
	virtual void OnAttach() override;
	virtual void OnDetach() override;


	void OnUpdate(Astan::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Astan::Event& event) override;
private:
	Astan::OrthographicCameraController m_CameraController;
	
	//Temp
	Astan::Ref<Astan::VertexArray> m_SquareVA;
	Astan::Ref<Astan::Shader> m_FlatColorShader;
	Astan::Ref<Astan::Texture2D> m_CheckerboardTexture;
	Astan::Ref<Astan::Texture2D> m_SpriteSheet;

	struct ProfileResult
	{
		const char* Name;
		float Time;
	};
	std::vector<ProfileResult> m_ProfileResults;

	glm::vec4 m_SquareColor = { 0.2f,0.3f,0.8f,1.0f };
	ParticleSystem m_ParticleSystem;
	ParticleProps m_Particle;
};