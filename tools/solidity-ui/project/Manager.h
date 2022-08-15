#pragma once

#include "Project.h"
#include "Testcases.h"
#include <memory>
#include <string>

namespace solidity::ui::project
{

class Manager
{
public:
	typedef std::vector<std::unique_ptr<solidity::ui::project::Project>> Projects;
	typedef std::deque<std::string> ProjectCache;

	static Manager& Instance()
	{
		static Manager manager;
		return manager;
	}

	void openProject(std::string const& _arguments);
	[[nodiscard]] ProjectCache history(int _count = -1, std::string const& _search = "") const;
	[[nodiscard]] Projects const& projects() const;
	void render();

	void load(std::string const& _filename);
	void save(std::string const& _filename);

	Testcases& testcases() { return m_testcases; }

private:
	Projects m_openedProjects;
	ProjectCache m_historicalProjects;
	Testcases m_testcases;
};

} // namespace solidity::ui::project
