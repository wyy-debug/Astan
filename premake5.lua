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
	include "vendor/premake"
	include "Astan/vendor/Box2D"
	include "Astan/vendor/GLFW"
	include "Astan/vendor/Glad"
	include "Astan/vendor/msdf-atlas-gen"
	include "Astan/vendor/imgui"
	include "Astan/vendor/yaml-cpp"
group ""

group "Core"
	include "Astan"
	include "Astan-ScriptCore"
group ""

group "Tools"
	include "Astannut"
group ""

group "Misc"
	include "Application"
group ""
