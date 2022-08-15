#include "Testcases.h"

#include "Manager.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <filesystem>
#include <iostream>

namespace solidity::ui::project
{

Testcases::Testcases()
{
	memset(m_search, 0, sizeof(m_search));

	std::filesystem::recursive_directory_iterator dir(
		std::filesystem::path(CMAKE_SOURCE_DIR "/test/libsolidity/semanticTests"));
	for (auto&& i: dir)
	{
		if (!is_directory(i))
		{
			std::string path{i.path().parent_path().generic_string()};
			boost::replace_all(path, CMAKE_SOURCE_DIR "/test/libsolidity/semanticTests/", "");
			boost::replace_all(path, CMAKE_SOURCE_DIR "/test/libsolidity/semanticTests", "");
			m_testcases[path].insert(i.path());
		}
	}
}

std::string Testcases::TrimMenuItem(std::string const& _string, size_t _maxLength)
{
	std::string result{_string};
	if (_string.length() > _maxLength)
		result = _string.substr(0, (_maxLength / 2)) + ".."
				 + _string.substr(_string.length() - (_maxLength / 2), _string.length());
	return result;
}

void Testcases::renderMenuItems(std::string const& _parent)
{
	(void) _parent;
	for (auto&& i: m_testcases)
	{
		if (boost::starts_with(i.first, _parent + "/"))
		{
			std::string last(i.first.substr(_parent.length() + 1));
			if (ImGui::BeginMenu(last.c_str()))
			{
				renderMenuItems(i.first);
				ImGui::EndMenu();
			}
		}
	}
	for (auto&& i: m_testcases)
		if (i.first == _parent)
		{
			for (auto&& file: i.second)
			{
				std::string entry = std::filesystem::path(file).filename().generic_string();
				if (ImGui::MenuItem(TrimMenuItem(entry, 32).c_str()))
					project::Manager::Instance().openProject(file);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					ImGui::SetTooltip("%s", file.c_str());
			}
		}
}

void Testcases::renderMenuItems()
{
	ImGui::InputTextWithHint(
		"##1", "<enter text to search test-case>", m_search, sizeof(m_search), ImGuiInputTextFlags_AutoSelectAll);
	std::string search(m_search);
	if (!search.empty())
	{
		for (auto&& i: m_testcases)
			for (auto&& file: i.second)
			{
				std::vector<std::string> words;
				boost::split(words, boost::to_lower_copy(search), [](char c) { return c == ' '; });
				bool containsAll = true;
				for (auto const& word: words)
					if (boost::to_lower_copy(std::filesystem::path(file).generic_string()).find(word)
						== std::string::npos)
						containsAll = false;
				if (containsAll)
				{
					std::string path{std::filesystem::path(file)};
					boost::replace_all(path, CMAKE_SOURCE_DIR "/test/libsolidity/semanticTests/", "");
					boost::replace_all(path, CMAKE_SOURCE_DIR "/test/libsolidity/semanticTests", "");
					if (ImGui::MenuItem(TrimMenuItem(path, 128).c_str()))
					{
						memset(m_search, 0, sizeof(m_search));
						project::Manager::Instance().openProject(file);
					}
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						ImGui::SetTooltip("%s", path.c_str());
				}
			}
	}
	else
	{
		for (auto&& i: m_testcases)
			if (!i.first.empty() && i.first.find("/") == std::string::npos && ImGui::BeginMenu(i.first.c_str()))
			{
				renderMenuItems(i.first);
				ImGui::EndMenu();
			}
		for (auto&& i: m_testcases)
			if (i.first.empty())
				for (auto&& file: i.second)
				{
					std::string entry = std::filesystem::path(file).filename().generic_string();
					if (ImGui::MenuItem(TrimMenuItem(entry, 32).c_str()))
						project::Manager::Instance().openProject(file);
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						ImGui::SetTooltip("%s", entry.c_str());
				}
	}
}

} // namespace solidity::ui::project
