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

using namespace solidity::util;
using namespace std;

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
			target = appendToRow(_targetRow, source->col, _factor * source->value)->next_in_row;
		else
			target = prependInRow(*target, source->col, _factor * source->value)->next_in_row;

		source = source->next_in_row;
	}
}


void SparseMatrix::appendRow(LinearExpression const& _entries)
{
	m_row_start.push_back(nullptr);
	m_row_end.push_back(nullptr);
	size_t row_nr = m_row_start.size() - 1;
	for (auto&& [i, v]: _entries.enumerate()) {
		if (!v)
			continue;
		appendToRow(row_nr, i, move(v));
	}
}

void SparseMatrix::remove(SparseMatrix::Entry& _e)
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

SparseMatrix::Entry* SparseMatrix::appendToRow(size_t _row, size_t _column, rational _value)
{
	// TODO could be combined with prependInRow
	// with successor being nullptr
	m_elements.emplace_back(make_unique<Entry>(Entry{
		move(_value),
		_row,
		_column,
		m_row_end[_row],
		nullptr,
		nullptr,
		nullptr
	}));
	Entry* e = m_elements.back().get();
	if (m_row_end[_row])
		m_row_end[_row]->next_in_row = e;
	m_row_end[_row] = e;

	if (!m_row_start[_row])
		m_row_start[_row] = e;

	adjustColumnProperties(*e);
	return e;
}

SparseMatrix::Entry* SparseMatrix::prependInRow(Entry& _successor, size_t _column, rational _value)
{
	size_t row = _successor.row;
	m_elements.emplace_back(make_unique<Entry>(Entry{
		move(_value),
		row,
		_column,
		_successor.prev_in_row,
		&_successor,
		nullptr,
		nullptr
	}));
	Entry* e = m_elements.back().get();
	_successor.prev_in_row = e;
	if (m_row_start[row] == &_successor)
		m_row_start[row] = e;

	adjustColumnProperties(*e);
	return e;
}

void SparseMatrix::adjustColumnProperties(Entry& _entry)
{
	size_t column = _entry.col;

	if (column >= m_col_start.size())
	{
		m_col_start.resize(column + 1);
		m_col_end.resize(column + 1);
	}
	Entry* c = nullptr;
	if (m_col_end[column] && m_col_end[column]->row > _entry.row)
	{
		c = m_col_start[column];
		// TODO could choose to search from end
		while (c && c->row < _entry.row)
			c = c->next_in_col;
	}
	_entry.next_in_col = c;
	if (c)
	{
		_entry.prev_in_col = c->prev_in_col;
		c->prev_in_col = &_entry;
	}
	else
	{
		_entry.prev_in_col = m_col_end[column];
		m_col_end[column] = &_entry;
	}
	if (_entry.prev_in_col)
		_entry.prev_in_col->next_in_col = &_entry;
	else
		m_col_start[column] = &_entry;
}
