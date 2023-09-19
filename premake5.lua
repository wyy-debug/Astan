workspace "Astan"
	architecture "x64" 

	startproject "Application"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Astan/vendor/GLFW/include"
IncludeDir["Glad"] = "Astan/vendor/Glad/include"
IncludeDir["ImGui"] = "Astan/vendor/imgui"
IncludeDir["glm"] = "Astan/vendor/glm"

include "Astan/vendor/GLFW"
include "Astan/vendor/Glad"
include "Astan/vendor/imgui"


project "Astan"
	location "Astan"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}") 
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "aspch.h"
	pchsource "Astan/src/aspch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/vendor\\spdlog\\include",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
	}
	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		"Dwmapi.lib"
	}
	filter "system:windows"
		systemversion "latest"

		defines
		{
			"AS_PLATFORM_WINDOWS",
			"AS_BUILD_DLL",
			"_WINDLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "AS_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "AS_RELEASE"
		runtime "Release"
		symbols "on"

	filter "configurations:Dist"
		defines "AS_DIST"
		runtime "Release"
		symbols "on"

project "Application"
	location "Application"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"


	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Astan/vendor\\spdlog\\include",
		"Astan/src",
		"Astan/vendor"		
		
	}

	links
	{
		"Astan"
	}

	filter "system:windows"

		systemversion "latest"

		defines
		{
			"AS_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "AS_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "AS_RELEASE"
		runtime "Release"
		symbols "on"

	filter "configurations:Dist"
		defines "AS_DIST"
		runtime "Release"
		symbols "on"