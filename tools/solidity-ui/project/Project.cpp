#include "Project.h"
#include "Editor.h"

namespace solidity::ui::project
{

void Project::initialize()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetWindowPos(
		{ImGui::GetTextLineHeightWithSpacing() + ImGui::GetTextLineHeightWithSpacing() * 2,
		 ImGui::GetTextLineHeightWithSpacing() * 2 + ImGui::GetTextLineHeightWithSpacing() * 2});
	ImGui::SetWindowSize({io.DisplaySize.x / 2, io.DisplaySize.y / 2});
	zep_init(Zep::NVec2f(1.0f, 1.0f));
	zep_load(Zep::ZepPath(CMAKE_SOURCE_DIR) / "LICENSE.txt");
}

void Project::renderProjectMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			if (ImGui::BeginMenu("Compiler Options"))
			{
				m_options_ui->render();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

bool Project::render()
{
	bool focused = false;
	if (m_opened)
	{
		if (ImGui::Begin(arguments().c_str(), &m_opened))
		{
			if (!m_initialized)
			{
				initialize();
				m_initialized = true;
			}
			focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

			zep_update();
			static Zep::NVec2i size = Zep::NVec2i(640, 480);
			zep_show(size);
		}
		ImGui::End();
	}
	return focused;
}

} // namespace solidity::ui::project
