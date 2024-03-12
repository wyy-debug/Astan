#pragma once

#include "Astan/Scene/SceneCamera.h"
#include "Astan/Renderer/Texture.h"
#include "Astan/Renderer/Font.h"
#include "Astan/Core/UUID.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <math.h>
#include <algorithm>

#define PI acos(-1)

namespace Astan
{
	struct IDComponent
	{
		UUID ID;
		
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f,0.0f,0.0f };
		glm::vec3 Rotation = { 0.0f,0.0f,0.0f };
		glm::vec3 Scale = { 1.0f,1.0f,1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f};
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};
	
	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f};
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};
	
	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	// Forward
	class ScriptableEntity;
	
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;
		
		ScriptableEntity*(*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() {return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) {delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	// Physics

	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic};
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;

	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		// TODO(Yan) : move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixtrue = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;

	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		// TODO(Yan) : move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixtrue = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;

	};

	struct TextComponent
	{
		std::string TextString;
		Ref<Font> FontAsset = Font::GetDefault();
		glm::vec4 Color{ 1.0f };
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;
	};


	struct PointLightComponent
	{
		glm::vec3 Position;
		glm::vec3 Fulx;

		float calculteRadius() const
		{
			const float INTENSITY_CUTOFF = 1.0f;
			const float ATTENTUATION_CUTOFF = 0.05f;
			glm::vec3 intensity = glm::vec3(Fulx.x / (4.0f * PI), Fulx.y / (4.0f * PI), Fulx.z / (4.0f * PI));
			float maxIntensity = glm::max(intensity.x, glm::max(intensity.y, intensity.z));
			float  attenuation = std::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;
			return 1.0f / sqrtf(attenuation);
		}
		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	struct AmbientLightComponent
	{
		glm::vec3 Irradiance;
		AmbientLightComponent() = default;
		AmbientLightComponent(const AmbientLightComponent&) = default;
	};

	struct PDirectionalLightComponent
	{
		glm::vec3 Direction;
		glm::vec3 Color;
		PDirectionalLightComponent() = default;
		PDirectionalLightComponent(const PDirectionalLightComponent&) = default;
	};

	struct RenderEntityComponent
	{
		glm::mat4 m_model_matrix{ glm::mat4(1.0f)};

		// mesh
		size_t                 m_MeshAssetId{ 0 };
		bool                   m_EnableVertexBlending{ false };
		std::vector<glm::mat4> m_JointMatrices;
		AxisAlignedBox         m_BoundingBox;

		// material
		size_t  m_Material_asset_id{ 0 };
		bool    m_Blend{ false };
		bool    m_DoubleSided{ false };
		glm::vec4 m_BaseColorFactor{ 1.0f, 1.0f, 1.0f, 1.0f };
		float   m_MetallicFactor{ 1.0f };
		float   m_RoughnessFactor{ 1.0f };
		float   m_NormalScale{ 1.0f };
		float   m_OcclusionStrength{ 1.0f };
		glm::vec3 m_EmissiveFactor{ 0.0f, 0.0f, 0.0f };
	};
	template<typename...Component>
	struct ComponentGroup
	{
	};

	using AllComponents = 
		ComponentGroup<TransformComponent, SpriteRendererComponent,
		CircleRendererComponent, CameraComponent, ScriptComponent,
		NativeScriptComponent,Rigidbody2DComponent, BoxCollider2DComponent,
		CircleCollider2DComponent, TextComponent, PointLightComponent, AmbientLightComponent, PDirectionalLightComponent, RenderEntityComponent>;

}