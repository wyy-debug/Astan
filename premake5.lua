workspace "Astan"
	architecture "x64" 


	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
		"%{prj.name}/src"
	}
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"AS_PLATFORM_WINDOWS",
			"AS_BUILD_DLL",
			"_WINDLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Application")
		}

	filter "configurations:Debug"
		defines "AS_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "AS_RELEASE"
		symbols "On"

	filter "configurations:Dist"
		defines "AS_DIST"
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
		symbols "On"

	filter "configurations:Release"
		defines "AS_RELEASE"
		symbols "On"

	filter "configurations:Dist"
		defines "AS_DIST"
		symbols "On"