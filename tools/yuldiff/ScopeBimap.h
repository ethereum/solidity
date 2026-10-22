/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libyul/YulName.h>

#include <map>
#include <vector>

namespace solidity::tools::cmpast
{

class ScopeBimap
{
public:
	class Scope
	{
	public:
		explicit Scope(ScopeBimap& _bimap): m_bimap{_bimap}
		{
			m_bimap.m_scopeStack.emplace_back();
		}

		~Scope()
		{
			for (auto const& [l, r] : m_bimap.m_scopeStack.back())
			{
				m_bimap.m_leftToRight.erase(l);
				m_bimap.m_rightToLeft.erase(r);
			}
			m_bimap.m_scopeStack.pop_back();
		}

	private:
		ScopeBimap& m_bimap;
	};

	/// Try to register a mapping l <-> r.
	/// Returns true if consistent (either new or already matches).
	bool tryMap(yul::YulName _l, yul::YulName _r)
	{
		auto itL = m_leftToRight.find(_l);
		auto itR = m_rightToLeft.find(_r);
		bool lMapped = itL != m_leftToRight.end();
		bool rMapped = itR != m_rightToLeft.end();

		if (lMapped && rMapped)
			return itL->second == _r && itR->second == _l;
		if (lMapped || rMapped)
			return false; // one side mapped but not to each other

		m_leftToRight[_l] = _r;
		m_rightToLeft[_r] = _l;
		m_scopeStack.back().emplace_back(_l, _r);
		return true;
	}

private:
	std::map<yul::YulName, yul::YulName> m_leftToRight;
	std::map<yul::YulName, yul::YulName> m_rightToLeft;
	std::vector<std::vector<std::pair<yul::YulName, yul::YulName>>> m_scopeStack;
};
}
