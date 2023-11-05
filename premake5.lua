workspace "Astan"
	architecture "x64" 

	startproject "Astannut"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Astan/vendor/GLFW/include"
IncludeDir["Glad"] = "Astan/vendor/Glad/include"
IncludeDir["ImGui"] = "Astan/vendor/imgui"
IncludeDir["glm"] = "Astan/vendor/glm"
IncludeDir["stb_image"] = "Astan/vendor/stb_image"
IncludeDir["entt"] = "Astan/vendor/entt/include"
IncludeDir["yamlcpp"] = "Astan/vendor/yaml-cpp/include"

group "Dependencies"
	include "Astan/vendor/GLFW"
	include "Astan/vendor/Glad"
	include "Astan/vendor/imgui"
	include "Astan/vendor/yaml-cpp"
group ""

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
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/glm/**.hpp",
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
		"%{IncludeDir.yamlcpp}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}"

	}
	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
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
		"Astan/vendor",
		"%{IncludeDir.glm}"	,
		"%{IncludeDir.entt}",
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

project "Astannut"
	location "Astannut"
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
		"Astan/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",

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