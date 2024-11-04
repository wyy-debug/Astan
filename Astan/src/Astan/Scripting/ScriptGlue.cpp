#include "aspch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Astan/Core/UUID.h"
#include "Astan/Core/KeyCodes.h"
#include "Astan/Core/Input.h"
#include "Astan/Scene/Scene.h"
#include "Astan/Scene/Entity.h"
#include "Astan/Physics/Physics2D.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/b2_body.h"

namespace Astan
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
#define AS_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Astan.InternalCalls::" #Name, Name) 

	static void NativeLog(MonoString* string, int parameter)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		std::cout << str << ", " << parameter << std::endl;
	}

	static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
	{
		AS_CORE_WARN("Value : {0}", *parameter);
		*outResult = glm::normalize(*parameter);
	}

	static float NativeLog_VectorDot(glm::vec3* parameter)
	{
		AS_CORE_WARN("Value : {0}", *parameter);
		return glm::dot(*parameter, *parameter);
	}
	
	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		*outTranslation =  entity.GetComponent<TransformComponent>().Translation;
	}
	
	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static void TransformComponent_GetScale(UUID entityID, glm::vec3* outScale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	static void TransformComponent_SetScale(UUID entityID, glm::vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		entity.GetComponent<TransformComponent>().Scale = *scale;
	}

	static MonoObject* GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	// Creare Entity
	static uint64_t Create_Entity(MonoString* string)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		char* cStr = mono_string_to_utf8(string);
		return scene->InternalCreateEntity(cStr);
	}

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		MonoType* managedType = mono_reflection_type_get_type(componentType);
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}
	
	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		AS_CORE_ASSERT(scene);
		Entity entity = scene->FindEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}

	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impuse, glm::vec2* point, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		AS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		AS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impuse->x, impuse->y), b2Vec2(point->x, point->y), wake);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID,glm::vec2* impulse,bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		AS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		AS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}
	static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2* outLinearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		AS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		AS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		*outLinearVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		AS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		AS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		return Utils::Rigidbody2DTypeFromBox2DBody(body->GetType());
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		AS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		AS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
	}

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	static void Add_SpriteRenderer(UUID uuid, glm::vec3 color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		scene->SpriteRendererAdd(uuid, color);
	}

	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypename = fmt::format("Astan.{}", structName);
				auto b = ScriptEngine::GetCoreAssemblyImage();
				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
					AS_CORE_ERROR("Could not find component type {}", managedTypename);
					return;
				}
				s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
			}(), ...);
	}

	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
	{
		AS_ADD_INTERNAL_CALL(NativeLog);
		AS_ADD_INTERNAL_CALL(NativeLog_Vector);
		AS_ADD_INTERNAL_CALL(NativeLog_VectorDot);

		// Entity
		AS_ADD_INTERNAL_CALL(Create_Entity);
		AS_ADD_INTERNAL_CALL(Entity_HasComponent);
		AS_ADD_INTERNAL_CALL(Entity_FindEntityByName);

		AS_ADD_INTERNAL_CALL(GetScriptInstance);

		AS_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		AS_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		AS_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		AS_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		AS_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		AS_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		AS_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		AS_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
		AS_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);

		AS_ADD_INTERNAL_CALL(Input_IsKeyDown);

		// SpriteRenderer

		AS_ADD_INTERNAL_CALL(Add_SpriteRenderer);
		
	}
}