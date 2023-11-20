#include "aspch.h"
#include "ContentBrowserPanel.h"
#include <imgui/imgui.h>

namespace Astan
{
	//Once we have projects, change this
	static const std::filesystem::path s_Assetspath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(s_Assetspath)
	{
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(s_Assetspath))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_Assetspath);
			std::string filenameString = relativePath.filename().string();
			if (directoryEntry.is_directory())
			{
				if (ImGui::Button(filenameString.c_str()))
				{
					m_CurrentDirectory /= path.filename();
				}
			}
			else 
			{
				if (ImGui::Button(filenameString.c_str()))
				{
				}
			}
		}

		ImGui::End();
	}
}