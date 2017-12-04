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
 * Optimiser component that changes the code so that it consists of a block starting with
 * a single block followed only by function definitions and with no functions defined
 * anywhere else.
 */

#include <libjulia/optimiser/FunctionHoister.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <libsolidity/interface/Exceptions.h>

#include <libdevcore/CommonData.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;


void FunctionHoister::operator()(Block& _block)
{
	bool topLevel = m_isTopLevel;
	m_isTopLevel = false;
	for (auto&& statement: _block.statements)
	{
		boost::apply_visitor(*this, statement);
		if (statement.type() == typeid(FunctionDefinition))
		{
			m_functions.emplace_back(std::move(statement));
			statement = Block{_block.location, {}};
		}
	}
	auto isEmptyBlock = [](Statement const& _st) -> bool {
		return _st.type() == typeid(Block) && boost::get<Block>(_st).statements.empty();
	};
	// Remove empty blocks
	boost::range::remove_erase_if(_block.statements, isEmptyBlock);
	if (topLevel)
		_block.statements += std::move(m_functions);
}
