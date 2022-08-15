#pragma once

#include <string>
#include <map>
#include <set>
namespace solidity::ui::project
{

class Testcases
{
public:
	Testcases();

	void renderMenuItems();

private:
	void renderMenuItems(std::string const& _parent);
	static std::string TrimMenuItem(std::string const& _string, size_t _maxLength);
	char m_search[128]{};
	std::map<std::string, std::set<std::string>> m_testcases;
};

}
