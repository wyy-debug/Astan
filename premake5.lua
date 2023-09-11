workspace "Astan"
	architecture "x64" 


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

include "Astan/vendor/GLFW"
include "Astan/vendor/Glad"

project "Astan"
	location "Astan"
	kind "SharedLib"
	language "C++"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "aspch.h"
	pchsource "Astan/src/aspch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/vendor\\spdlog\\include",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}"

	}
	links
	{
		"GLFW",
		"Glad",
		"opengl32.lib",
		"Dwmapi.lib"
	}
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"AS_PLATFORM_WINDOWS",
			"AS_BUILD_DLL",
			"_WINDLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Application")
		}

	filter "configurations:Debug"
		defines "AS_DEBUG"
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "AS_RELEASE"
		buildoptions "/MD"
		symbols "On"

	filter "configurations:Dist"
		defines "AS_DIST"
		buildoptions "/MD"
		symbols "On"

project "Application"
	location "Application"
	kind "ConsoleApp"
	language "C++"

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
		"Astan/src"
	}

	links
	{
		"Astan"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"AS_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "AS_DEBUG"
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "AS_RELEASE"
		buildoptions "/MD"
		symbols "On"

	filter "configurations:Dist"
		defines "AS_DIST"
		buildoptions "/MD"
		symbols "On"