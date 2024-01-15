#include "aspch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Astan/Core/UUID.h"
#include "Astan/Scene/Scene.h"
#include "Astan/Scene/Entity.h"

#include "mono/metadata/object.h"

namespace Astan
{

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
	
	static void Entity_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		*outTranslation =  entity.GetComponent<TransformComponent>().Translation;
	}
	
	static void Entity_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	void ScriptGlue::RegisterFunctions()
	{
		AS_ADD_INTERNAL_CALL(NativeLog);
		AS_ADD_INTERNAL_CALL(NativeLog_Vector);
		AS_ADD_INTERNAL_CALL(NativeLog_VectorDot);
		AS_ADD_INTERNAL_CALL(Entity_GetTranslation);
		AS_ADD_INTERNAL_CALL(Entity_SetTranslation);
	}
}