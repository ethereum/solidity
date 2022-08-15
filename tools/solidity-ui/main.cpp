#include <SDL.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <imgui.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include "assets/JetBrainsMono-Regular.ttf.h"
#include "libsolidity/interface/CompilerStack.h"
#include "libsolidity/interface/DebugSettings.h"
#include "project/Manager.h"
#include "project/Project.h"
#include "solc/CommandLineParser.h"
#include "solidity/BuildInfo.h"
#include "ui/CommandLineOptions.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
#include <memory>
#include <vector>

std::string createTooltipString(std::string const& string)
{
	std::string result;
	size_t length;
	for (auto && ch : string)
	{
		result += ch;
		++length;
		if (length > 100)
		{
			result += "\n       ";
			length = 0;
		}
	}
	return boost::trim_copy(result);
}

int main(int argc, char* argv[])
{
	(void) argc;
	(void) argv;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char* glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	auto window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	//	auto window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_Window* window
		= SDL_CreateWindow("solidity-ui", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	auto font_data = new uint8_t[sizeof(solidity::ui::assets::JetBrainsMono_Regular_ttf)];
	memcpy(
		font_data,
		solidity::ui::assets::JetBrainsMono_Regular_ttf,
		sizeof(solidity::ui::assets::JetBrainsMono_Regular_ttf));
	// ownership of font_data will be transferred imgui
	io.Fonts
		->AddFontFromMemoryTTF(font_data, sizeof(font_data), 16 * ImGui::GetPlatformIO().Monitors.begin()->DpiScale);

	// #2b2b2b
	ImVec4 clear_color = ImVec4(0.169f, 0.169f, 0.169f, 1.00f);
	// ImVec4 inactive_color = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);

	char solc_parameters[512];
	memset(solc_parameters, 0, sizeof(solc_parameters));
	char search_term[128];
	memset(search_term, 0, sizeof(search_term));

	std::string projectManagerConfig = boost::filesystem::current_path().generic_string() + "/solidity-ui.json";
	solidity::ui::project::Manager& projectManager = solidity::ui::project::Manager::Instance();
	projectManager.load(projectManagerConfig);

	// Main loop
	bool done = false;
	while (!done)
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
		// inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
		// clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
		// clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and
		// hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE
				&& event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		bool running = !done;
		ImGui::Begin(
			"solidity-ui",
			&running,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
				| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
		ImGui::SetWindowPos(ImVec2{0, 0});
		ImGui::SetWindowSize(io.DisplaySize);

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Solidity UI"))
			{
				ImGui::Separator();
				ImGui::Text("Load Project");
				ImGui::Text(" > via CLI");
				ImGui::Text("   $ solc ");
				ImGui::SameLine();
				if (ImGui::InputTextWithHint(
						"##0",
						"<command line parameters>",
						(char*) &solc_parameters,
						sizeof(solc_parameters),
						ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
				{
					memset(search_term, 0, sizeof(search_term));
					projectManager.openProject(std::string(solc_parameters));
				}
				if (ImGui::BeginMenu(" > from Testcases"))
				{
					projectManager.testcases().renderMenuItems();
					ImGui::EndMenu();
				}
				ImGui::Separator();
				if (!projectManager.history().empty())
				{
					ImGui::Text("Project History");
					ImGui::Text("          ");
					ImGui::SameLine();
					ImGui::InputTextWithHint(
						"##1",
						"<enter text to search history>",
						search_term,
						sizeof(search_term),
						ImGuiInputTextFlags_AutoSelectAll);
				}
				std::string search(search_term);
				solidity::ui::project::Manager::ProjectCache cache = projectManager.history(5, search);
				for (auto& project: cache)
				{
					std::string project_simplified{project};
					boost::replace_all(
						project_simplified, CMAKE_SOURCE_DIR "/test/libsolidity/semanticTests/", "${semanticTests}/");
					boost::replace_all(
						project_simplified, CMAKE_SOURCE_DIR "/test/libsolidity/semanticTests", "${semanticTests}");
					if (search.empty())
					{
						if (project_simplified.length() > 32)
							project_simplified
								= "[..]"
								  + project_simplified
										.substr(project_simplified.length() - 32, project_simplified.length());
					}
					if (ImGui::MenuItem((" $ solc " + project_simplified).c_str(), nullptr))
					{
						memset(search_term, 0, sizeof(search_term));
						projectManager.openProject(project);
					}
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						ImGui::SetTooltip("%s", createTooltipString(" $ solc " + project).c_str());
				}
				if (!projectManager.history().empty())
				{
					if (ImGui::BeginMenu("   Full Project History"))
					{
						for (auto& project: projectManager.history())
						{
							std::string project_simplified{project};
							boost::replace_all(
								project_simplified,
								CMAKE_SOURCE_DIR "/test/libsolidity/semanticTests",
								"${semanticTests}");
							if (project_simplified.length() > 64)
								project_simplified
									= "[..]"
									  + project_simplified
										  .substr(project_simplified.length() - 64, project_simplified.length());
							if (ImGui::MenuItem((" $ solc " + project_simplified).c_str(), nullptr))
								projectManager.openProject(project);
							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
								ImGui::SetTooltip("%s", createTooltipString(" $ solc " + project).c_str());
						}
						ImGui::EndMenu();
					}
					ImGui::Separator();
				}
				if (ImGui::MenuItem("Exit"))
				{
					done = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		ImGui::End();

		projectManager.render();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("v" ETH_PROJECT_VERSION "-" SOL_VERSION_PRERELEASE "+" SOL_VERSION_BUILDINFO, false))
				ImGui::EndMenu();
			ImGui::EndMainMenuBar();
		}

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
		glClearColor(
			clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	projectManager.save(projectManagerConfig);

	return 0;
}
