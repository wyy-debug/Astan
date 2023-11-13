include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"
workspace "Astan"
	architecture "x64" 

	startproject "Astannut"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Astan/vendor/GLFW"
	include "Astan/vendor/Glad"
	include "Astan/vendor/imgui"
	include "Astan/vendor/yaml-cpp"
group ""

include "Astan"
include "Application"
include "Astannut"