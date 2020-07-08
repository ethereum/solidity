// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/formal/SSAVariable.h>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::frontend::smt;

SSAVariable::SSAVariable()
{
	resetIndex();
}

void SSAVariable::resetIndex()
{
	m_currentIndex = 0;
	m_nextFreeIndex = 1;
}

void SSAVariable::setIndex(unsigned _index)
{
	m_currentIndex = _index;
	if (m_nextFreeIndex <= _index)
		m_nextFreeIndex = _index + 1;
}
