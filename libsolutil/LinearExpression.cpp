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

#include <libsolutil/LinearExpression.h>

using solidity::util;

void SparseMatrix::multiplyRowByFactor(size_t _row, rational const& _factor)
{
	Entry* e = m_row_start[_row];
	while (e)
	{
		e->value *= _factor;
		Entry* next = e->next_in_row;
		if (!e->value)
			remove(*e);
		e = next;
	}
}

void SparseMatrix::addMultipleOfRow(size_t _sourceRow, size_t _targetRow, rational const& _factor)
{
	Entry* source = m_row_start[_sourceRow];
	Entry* target = m_row_start[_targetRow];

	while (source)
	{
		while (target && target->col < source->col)
			target = target->next_in_row;
		if (target && target->col == source->col)
		{
			target->value += _factor * source->value;
			if (!target->value)
			{
				Entry* next = target->next_in_row;
				remove(*target);
				target = next;
			}
		}
		else if (!target)
			target = appendToRow(_targetRow, source->col, _factor * source->value);
		else if (target->col > source->col)
			target = prependInRow(target, source->col, _factor * source->value);

		source = source->next_in_row;
	}
}


void SparseMatrix::appendRow(LinearExpression const& _entries)
{
	Entry* prev = nullptr;
	m_row_start.push(nullptr);
	m_row_end.push(nullptr);
	size_t row_nr = m_row_start.size() - 1;
	for (auto&& [i, v]: _entries.enumerate()) {
		if (!v)
			continue;
		prev = appendToRow(row_nr, i, move(v));

		prev = curr;
	}
}

void SparseMatrix::remove(Entry& _e)
{
	if (_e.prev_in_row)
		_e.prev_in_row->next_in_row = _e.next_in_row;
	else
		m_row_start[_e.row] = _e.next_in_row;
	if (_e.next_in_row)
		_e.next_in_row->prev_in_row = _e.prev_in_row;
	else
		m_row_end[_e.row] = _e.prev_in_row;
	if (_e.prev_in_col)
		_e.prev_in_col->next_in_col = _e.next_in_col;
	else
		m_col_start[_e.col] = _e.next_in_col;
	if (_e.next_in_col)
		_e.next_in_col->prev_in_col = _e.prev_in_col;
	else
		m_col_end[_e.col] = _e.prev_in_col;
}

void SparseMatrix::appendToRow(size_t _row, size_t _column, rational _value)
{
	m_elements.emplace(make_unique<Element>(
		move(_value),
		_row,
		_column,
		m_row_end[_row],
		nullptr,
		m_column_end[i],
		nullptr
	));
	Entry const* e = m_elements.back().get();
	if (m_row_end[_row])
		m_row_end[_row]->next_in_row = e;
	if (!m_row_start[_row])
		m_row_start[_row] = e;
	if (i >= m_col_start.size())
		m_col_start.resize(i + 1);
	if (!m_col_start[i])
		m_col_start[i] = e;
	if (i >= m_col_end.size())
		m_col_end.resize(i + 1);
	if (!m_col_end[i])
		m_col_end[i] = e;
}
