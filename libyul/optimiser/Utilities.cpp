/*(
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

#include <libyul/optimiser/Utilities.h>

#include <libyul/AsmData.h>
#include <libyul/Exceptions.h>

#include <libdevcore/CommonData.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace dev;
using namespace yul;

void yul::removeEmptyBlocks(Block& _block)
{
	auto isEmptyBlock = [](Statement const& _st) -> bool {
		return _st.type() == typeid(Block) && boost::get<Block>(_st).statements.empty();
	};
	boost::range::remove_erase_if(_block.statements, isEmptyBlock);
}

u256 yul::valueOfNumberLiteral(Literal const& _literal)
{
	assertThrow(_literal.kind == LiteralKind::Number, OptimizerException, "");
	std::string const& literalString = _literal.value.str();
	assertThrow(isValidDecimal(literalString) || isValidHex(literalString), OptimizerException, "");
	return u256(literalString);
}
