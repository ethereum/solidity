#include <libyul/optimiser/VariableAssignmentMap.h>

using std::set;
using namespace solidity::yul;

void VariableAssignmentMap::insert(YulString const& _variable, set<YulString> const& _references)
{
	for (auto&& reference: _references)
	{
		m_ordered[_variable.id()].emplace(reference);
		m_reversed[reference.id()].emplace(_variable);
	}
}

void VariableAssignmentMap::erase(YulString const& _variable)
{
	for (auto&& reference: m_ordered[_variable.id()])
		if (m_reversed.find(reference.id()) != m_reversed.end())
		{
			if (m_reversed[reference.id()].size() > 1)
				m_reversed[reference.id()].erase(_variable);
			else
				// Only fully remove an entry if no variables other than _variable
				// are contained in the set pointed to by reference.
				m_reversed.erase(reference.id());
		}
	m_ordered.erase(_variable.id());
}

set<YulString> const* VariableAssignmentMap::getOrderedOrNullptr(YulString const& _variable) const
{
	auto&& it = m_ordered.find(_variable.id());
	return (it != m_ordered.end()) ? &it->second : nullptr;
}

set<YulString> const* VariableAssignmentMap::getReversedOrNullptr(YulString const& _variable) const
{
	auto&& it = m_reversed.find(_variable.id());
	return (it != m_reversed.end()) ? &it->second : nullptr;
}
