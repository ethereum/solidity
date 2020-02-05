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

#pragma once

#include <cassert>
#include <vector>

namespace solidity::phaser
{

class Selection
{
public:
	Selection() = default;
	Selection(Selection const&) = delete;
	Selection& operator=(Selection const&) = delete;
	virtual ~Selection() = default;

	virtual std::vector<size_t> materialize(size_t _poolSize) const = 0;
};

class RangeSelection: public Selection
{
public:
	explicit RangeSelection(double _startPercent = 0.0, double _endPercent = 1.0):
		m_startPercent(_startPercent),
		m_endPercent(_endPercent)
	{
		assert(0 <= m_startPercent && m_startPercent <= m_endPercent && m_endPercent <= 1.0);
	}

	std::vector<size_t> materialize(size_t _poolSize) const override;

private:
	double m_startPercent;
	double m_endPercent;
};

class RandomSelection: public Selection
{
public:
	explicit RandomSelection(double _selectionSize):
		m_selectionSize(_selectionSize)
	{
		assert(_selectionSize >= 0);
	}

	std::vector<size_t> materialize(size_t _poolSize) const override;

private:
	double m_selectionSize;
};

}
