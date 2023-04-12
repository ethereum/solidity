#include <libyul/optimiser/VariableAssignmentMap.h>

using std::set;
using namespace solidity::yul;

void VariableAssignmentMap::insert(YulString const& _variable, std::set<YulString> const& _references)
{
	erase(_variable);
	m_ordered[_variable] = _references;
	for (auto&& reference: _references)
		m_reversed[reference].emplace(_variable);
}

void VariableAssignmentMap::erase(YulString const& _variable)
{
	for (auto&& reference: m_ordered[_variable])
		if (m_reversed.find(reference) != m_reversed.end())
		{
			if (m_reversed[reference].size() > 1)
				m_reversed[reference].erase(_variable);
			else
				// Only fully remove an entry if no variables other than _variable
				// are contained in the set pointed to by reference.
				m_reversed.erase(reference);
		}
	m_ordered.erase(_variable);
}

set<YulString> const* VariableAssignmentMap::getOrderedOrNullptr(YulString const& _variable) const
{
	auto&& it = m_ordered.find(_variable);
	return (it != m_ordered.end()) ? &it->second : nullptr;
}

set<YulString> const* VariableAssignmentMap::getReversedOrNullptr(YulString const& _variable) const
{
	auto&& it = m_reversed.find(_variable);
	return (it != m_reversed.end()) ? &it->second : nullptr;
}
