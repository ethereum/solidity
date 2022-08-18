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

SparseMatrix::SparseMatrixIterator SparseMatrix::IteratorCombiner::begin()
{
	if (
		(m_isRow && m_rowOrColumn >= m_matrix.m_row_start.size()) ||
		(!m_isRow && m_rowOrColumn >= m_matrix.m_col_start.size())
	)
		return SparseMatrixIterator(nullptr, m_isRow);
	else
		return SparseMatrixIterator(
			m_isRow ? m_matrix.m_row_start[m_rowOrColumn] : m_matrix.m_col_start[m_rowOrColumn],
			m_isRow
		);
}

SparseMatrix::SparseMatrixIterator SparseMatrix::IteratorCombiner::end()
{
	return SparseMatrixIterator(nullptr, m_isRow);
}

SparseMatrix::IteratorCombiner SparseMatrix::iterateColumn(size_t _column)
{
	return IteratorCombiner{
		_column,
		false,
		*this
	};
}

SparseMatrix::IteratorCombiner SparseMatrix::iterateRow(size_t _row)
{
	return IteratorCombiner{
		_row,
		true,
		*this
	};
}

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

void SparseMatrix::addMultipleOfRow(size_t _sourceRow, size_t _targetRow, rational const _factor)
{
	ensureSize(_targetRow, 0);
	solAssert(_sourceRow != _targetRow);
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
			else
				target = target->next_in_row;
		}
		else if (rational newValue = _factor * source->value)
			target = prependInRow(target, _targetRow, source->col, newValue)->next_in_row;
		else if (target)
			target = target->next_in_row;

		source = source->next_in_row;
	}
}

SparseMatrix::Entry& SparseMatrix::entry(size_t _row, size_t _column)
{
	ensureSize(_row, _column);
	Entry* successor = entryOrSuccessorInRow(_row, _column);
	if (successor && successor->col == _column)
		return *successor;
	else
		return *prependInRow(successor, _row, _column, {});
}

void SparseMatrix::remove(SparseMatrix::Entry& _e)
{
	// TODO this does not deallocate the entry.
	// At some point we should perform cleanup
	// and swap entries in the vector
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

void SparseMatrix::appendRow(LinearExpression const& _entries)
{
	size_t row_nr = rows();
	ensureSize(row_nr, 0);
	for (auto&& [i, v]: _entries.enumerate())
	{
		if (!v)
			continue;
		prependInRow(nullptr, row_nr, i, move(v));
	}
}

void SparseMatrix::ensureSize(size_t _row, size_t _column)
{
	if (_column >= m_col_start.size())
	{
		m_col_start.resize(_column + 1);
		m_col_end.resize(_column + 1);
	}
	if (_row >= m_row_start.size())
	{
		m_row_start.resize(_row + 1);
		m_row_end.resize(_row + 1);
	}
}

SparseMatrix::Entry* SparseMatrix::entryOrSuccessorInRow(size_t _row, size_t _column)
{
	Entry* successor = nullptr;
	if (m_row_end[_row] && m_row_end[_row]->col >= _column)
	{
		successor = m_row_start[_row];
		// TODO could choose to search from end
		while (successor && successor->col < _column)
			successor = successor->next_in_row;
	}
	return successor;
}

SparseMatrix::Entry* SparseMatrix::prependInRow(Entry* _successor, size_t _row, size_t _column, rational _value)
{
	m_elements.emplace_back(make_unique<Entry>(Entry{
		move(_value),
		_row,
		_column,
		nullptr,
		_successor,
		nullptr,
		nullptr
	}));
	Entry* e = m_elements.back().get();
	if (_successor)
	{
		e->prev_in_row = _successor->prev_in_row;
		_successor->prev_in_row = e;
	}
	else
	{
		e->prev_in_row = m_row_end[_row];
		m_row_end[_row] = e;
	}
	if (e->prev_in_row)
		e->prev_in_row->next_in_row = e;
	else
		m_row_start[_row] = e;

	adjustColumnProperties(*e);
	return e;
}

void SparseMatrix::adjustColumnProperties(Entry& _entry)
{
	size_t column = _entry.col;

	solAssert(column < m_col_start.size());
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
