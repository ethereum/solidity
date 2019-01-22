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
/**
 * Some useful snippets for the optimiser.
 */

#include <libyul/Utilities.h>

#include <libyul/AsmData.h>
#include <libyul/Exceptions.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

u256 yul::valueOfNumberLiteral(Literal const& _literal)
{
	assertThrow(_literal.kind == LiteralKind::Number, OptimizerException, "");
	std::string const& literalString = _literal.value.str();
	assertThrow(isValidDecimal(literalString) || isValidHex(literalString), OptimizerException, "");
	return u256(literalString);
}

template<>
bool Less<Literal>::operator()(Literal const& _lhs, Literal const& _rhs) const
{
	if (std::make_tuple(_lhs.kind, _lhs.type) != std::make_tuple(_rhs.kind, _rhs.type))
		return std::make_tuple(_lhs.kind, _lhs.type) < std::make_tuple(_rhs.kind, _rhs.type);

	if (_lhs.kind == LiteralKind::Number)
		return valueOfNumberLiteral(_lhs) < valueOfNumberLiteral(_rhs);
	else
		return _lhs.value < _rhs.value;
}
