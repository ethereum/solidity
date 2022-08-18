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

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>
#include <liblangutil/Exceptions.h>

#include <boost/rational.hpp>

#include <range/v3/view/tail.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/algorithm/all_of.hpp>

#include <optional>
#include <stack>


namespace solidity::util
{

using rational = boost::rational<bigint>;

/**
 * A linear expression of the form
 * factors[0] + factors[1] * X1 + factors[2] * X2 + ...
 * The set and order of variables is implied.
 */
class LinearExpression
{
public:
	/// Creates the expression "_factor * X_index"
	static LinearExpression factorForVariable(size_t _index, rational _factor)
	{
		LinearExpression result;
		result.resize(_index + 1);
		result[_index] = std::move(_factor);
		return result;
	}

	static LinearExpression constant(rational _factor)
	{
		LinearExpression result;
		result.resize(1);
		result[0] = std::move(_factor);
		return result;
	}

	rational const& get(size_t _index) const
	{
		static rational const zero;
		return _index < factors.size() ? factors[_index] : zero;
	}

	rational const& operator[](size_t _index) const
	{
		return factors[_index];
	}

	rational& operator[](size_t _index)
	{
		return factors[_index];
	}

	auto enumerate() const { return factors | ranges::view::enumerate; }
	// leave out the zero if exists
	auto enumerateTail() const { return factors | ranges::view::enumerate | ranges::view::tail; }

	rational const& front() const { return factors.front(); }

	void push_back(rational _value) { factors.push_back(std::move(_value)); }

	size_t size() const { return factors.size(); }

	void resize(size_t _size, rational _default = {})
	{
		factors.resize(_size, std::move(_default));
	}

	/// @returns true if all factors of variables are zero.
	bool isConstant() const
	{
		return ranges::all_of(factors | ranges::views::tail, [](rational const& _v) { return !_v; });
	}

	LinearExpression& operator/=(rational const& _divisor)
	{
		for (rational& x: factors)
			if (x)
				x /= _divisor;
		return *this;
	}

	LinearExpression& operator*=(rational const& _factor)
	{
		for (rational& x: factors)
			if (x)
				x *= _factor;
		return *this;
	}

	friend LinearExpression operator*(rational const& _factor, LinearExpression _expr)
	{
		for (rational& x: _expr.factors)
			if (x)
				x *= _factor;
		return _expr;
	}

	LinearExpression& operator-=(LinearExpression const& _y)
	{
		if (size() < _y.size())
			resize(_y.size());
		for (size_t i = 0; i < size(); ++i)
			if (i < _y.size() && _y[i])
				(*this)[i] -= _y[i];
		return *this;
	}

	LinearExpression operator-(LinearExpression const& _y) const
	{
		LinearExpression result = *this;
		result -= _y;
		return result;
	}

	LinearExpression& operator+=(LinearExpression const& _y)
	{
		if (size() < _y.size())
			resize(_y.size());
		for (size_t i = 0; i < size(); ++i)
			if (i < _y.size() && _y[i])
				(*this)[i] += _y[i];
		return *this;
	}

	LinearExpression operator+(LinearExpression const& _y) const
	{
		LinearExpression result = *this;
		result += _y;
		return result;
	}


	/// Multiply two linear expression. This only works if at least one of them is a constant.
	/// Returns nullopt otherwise.
	friend std::optional<LinearExpression> operator*(
		std::optional<LinearExpression> _x,
		std::optional<LinearExpression> _y
	)
	{
		if (!_x || !_y)
			return std::nullopt;
		if (!_y->isConstant())
			swap(_x, _y);
		if (!_y->isConstant())
			return std::nullopt;

		rational const& factor = _y->get(0);

		for (rational& element: _x->factors)
			element *= factor;
		return _x;
	}

private:
	std::vector<rational> factors;
};


class SparseMatrix
{
public:
	struct Entry
	{
		rational value;
		// TOOD make it 32 bit as well
		size_t row;
		size_t col;
		// TODO maybe better to use 32-bit indices instead of 64-bit pointers
		Entry* prev_in_row;
		Entry* next_in_row;
		Entry* prev_in_col;
		Entry* next_in_col;
	};
	struct SparseMatrixIterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = Entry;
		using pointer           = Entry*;
		using reference         = Entry&;

		SparseMatrixIterator(pointer _ptr, bool _isRow): m_ptr(_ptr), m_isRow(_isRow) {}

		reference operator*() const { return *m_ptr; }
		pointer operator->() { return m_ptr; }
		SparseMatrixIterator& operator++()
		{
			m_ptr = m_isRow ? m_ptr->next_in_row : m_ptr->next_in_col;
			return *this;
		}
		SparseMatrixIterator operator++(int) { SparseMatrixIterator tmp = *this; ++(*this); return tmp; }
		friend bool operator==(SparseMatrixIterator const& _a, SparseMatrixIterator const& _b)
		{
			return _a.m_ptr == _b.m_ptr && _a.m_isRow == _b.m_isRow;
		}
		friend bool operator!=(SparseMatrixIterator const& _a, SparseMatrixIterator const& _b)
		{
			return _a.m_ptr != _b.m_ptr || _a.m_isRow != _b.m_isRow;
		}

	private:
		Entry* m_ptr;
		bool m_isRow;
	};
	struct IteratorCombiner
	{
		size_t m_rowOrColumn;
		bool m_isRow;
		SparseMatrix& m_matrix;
		SparseMatrixIterator begin();
		SparseMatrixIterator end();
	};

	size_t rows() const { return m_row_start.size(); }
	size_t columns() const { return m_col_start.size(); }

	/// @returns Entry for all non-zero v in the column _column
	IteratorCombiner iterateColumn(size_t _column);
	/// @returns Entry for all non-zero v in the row _row
	IteratorCombiner iterateRow(size_t _row);
	void multiplyRowByFactor(size_t _row, rational const& _factor);
	void addMultipleOfRow(size_t _sourceRow, size_t _targetRow, rational const& _factor);
	Entry& entry(size_t _row, size_t _column);
	/// Inserts the value at the row/rolumn.
	/// Assumes the entry does not exist yet.
	void insert(size_t _row, size_t _column, rational _value);

	void appendRow(LinearExpression const& _entries);

private:
	void ensureSize(size_t _row, size_t _column);
	/// @returns the entry at the row/column if it exists or its successor in the row.
	Entry* entryOrSuccessorInRow(size_t _row, size_t _column);

	void remove(Entry& _entry);
	/// Prepends a new entry before the given element or at end of row if nullptr.
	Entry* prependInRow(Entry* _successor, size_t _row, size_t _column, rational _value);
	void adjustColumnProperties(Entry& _entry);

	// TODO unique_ptr?
	std::vector<std::shared_ptr<Entry>> m_elements;
	std::vector<Entry*> m_row_start;
	std::vector<Entry*> m_col_start;
	std::vector<Entry*> m_row_end;
	std::vector<Entry*> m_col_end;
};

}
