#pragma once

#include "ui/CommandLineOptions.h"
#include <imgui.h>
#include <string>
#include <utility>

namespace solidity::ui::project
{

class Project
{
public:
	explicit Project(std::string _arguments): m_arguments(std::move(_arguments))
	{
		m_options_ui = std::make_unique<solidity::ui::CommandLineOptions>(m_options);
	}

	[[nodiscard]] std::string const& arguments() const { return m_arguments; }

	bool render();

	void renderProjectMenu();

	[[nodiscard]] bool opened() const { return m_opened; }

private:
	void initialize();

	bool m_initialized{false};
	bool m_opened{true};
	std::string m_arguments;

	solidity::frontend::CommandLineOptions m_options;
	std::unique_ptr<solidity::ui::CommandLineOptions> m_options_ui;
};

} // namespace solidity::ui::project
