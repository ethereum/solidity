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

#include <libsmtutil/SolverInterface.h>

namespace solidity::smtutil
{

/// Signed division in SMTLIB2 rounds differently than EVM.
/// This does not check for division by zero!
inline Expression signedDivisionEVM(Expression _left, Expression _right)
{
	return Expression::ite(
		_left >= 0,
		Expression::ite(_right >= 0, _left / _right, 0 - (_left / (0 - _right))),
		Expression::ite(_right >= 0, 0 - ((0 - _left) / _right), (0 - _left) / (0 - _right))
	);
}

inline Expression abs(Expression _value)
{
	return Expression::ite(_value >= 0, _value, 0 - _value);
}

/// Signed modulo in SMTLIB2 behaves differently with regards
/// to the sign than EVM.
/// This does not check for modulo by zero!
inline Expression signedModuloEVM(Expression _left, Expression _right)
{
	return Expression::ite(
		_left >= 0,
		_left % _right,
		Expression::ite(
			(_left % _right) == 0,
			0,
			(_left % _right) - abs(_right)
		)
	);
}

}
