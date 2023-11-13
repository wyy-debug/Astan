project "Application"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Astan/vendor/spdlog/include",
		"%{wks.location}/Astan/src",
		"%{wks.location}/Astan/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	links
	{
		"Astan"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "AS_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "AS_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "AS_DIST"
		runtime "Release"
		optimize "on"
