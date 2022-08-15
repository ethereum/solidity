#include "Manager.h"
#include "3rd/json.hpp"
#include <boost/algorithm/string.hpp>
#include <fstream>

namespace solidity::ui::project
{

void Manager::openProject(std::string const& _arguments)
{
	auto projectIterator = std::find(m_historicalProjects.begin(), m_historicalProjects.end(), _arguments);
	if (projectIterator != m_historicalProjects.end())
		m_historicalProjects.erase(projectIterator);
	m_historicalProjects.emplace_front(_arguments);
	m_openedProjects.emplace_back(std::make_unique<solidity::ui::project::Project>(_arguments));
}

Manager::ProjectCache Manager::history(int _count /*=-1*/, std::string const& _search /*=""*/) const
{
	Manager::ProjectCache result;
	std::vector<std::string> words;
	boost::split(words, _search, [](char c) { return c == ' '; });
	for (auto const& project: m_historicalProjects)
	{
		bool containsAll = true;
		for (auto const& word: words)
			if (project.find(word) == std::string::npos)
				containsAll = false;
		if (containsAll)
			result.emplace_back(project);
	}
	if (_count != -1 && result.size() > static_cast<size_t>(_count))
		result.erase(result.begin() + _count, result.begin() + static_cast<int>(result.size()));
	return result;
}

Manager::Projects const& Manager::projects() const { return m_openedProjects; }

void Manager::load(std::string const& _filename)
{
	m_historicalProjects.clear();
	m_openedProjects.clear();
	nlohmann::json json;
	try
	{
		std::ifstream f(_filename);
		json = nlohmann::json::parse(f);
		for (auto const& item: json["history"])
			m_historicalProjects.emplace_back(item);
	}
	catch (...)
	{
	}
}

void Manager::save(std::string const& _filename)
{
	nlohmann::json json;
	nlohmann::json& historicalProjects = json["history"];
	for (const auto& project: history())
		historicalProjects.push_back(project);
	try
	{
		std::ofstream o(_filename);
		o << std::setw(4) << json << std::endl;
	}
	catch (...)
	{
	}
}

void Manager::render()
{
	static solidity::ui::project::Project* lastFocusedProject{nullptr};
	bool projectOpened = false;
	for (auto&& project: m_openedProjects)
	{
		if (project->opened())
			projectOpened = true;
		if (project->render())
			lastFocusedProject = project.get();
	}

	if (projectOpened == false)
		lastFocusedProject = nullptr;
	if (lastFocusedProject)
		lastFocusedProject->renderProjectMenu();
}

} // namespace solidity::ui::project
