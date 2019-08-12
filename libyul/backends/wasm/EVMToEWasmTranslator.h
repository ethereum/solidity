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
 * Translates Yul code from EVM dialect to eWasm dialect.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/Dialect.h>

namespace yul
{
struct Object;

class EVMToEWasmTranslator: public ASTModifier
{
public:
	EVMToEWasmTranslator(Dialect const& _evmDialect): m_dialect(_evmDialect) {}
	Object run(Object const& _object);

private:
	void parsePolyfill();

	Dialect const& m_dialect;

	std::shared_ptr<Block> m_polyfill;
	std::set<YulString> m_polyfillFunctions;
};

}
