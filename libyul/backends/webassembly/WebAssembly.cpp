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
 * @author Alex Beregszaszi
 * @date 2017
 * Yul to WebAssembly code generator.
 */

#include <libyul/backends/webassembly/WebAssembly.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/interface/Exceptions.h>

#include <libdevcore/CommonIO.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

class Generator: public boost::static_visitor<>
{
public:
	/// Create the code transformer which appends assembly to _state.assembly when called
	/// with parsed assembly data.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	explicit Generator(assembly::Block const& _block)
	{
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));
	}

	string assembly() { return m_assembly; }

public:
	void operator()(assembly::Instruction const&)
	{
		solAssert(false, "Instructions are not supported in Yul.");
	}
	void operator()(assembly::FunctionalInstruction const&)
	{
		solAssert(false, "Instructions are not supported in Yul.");
	}
	void operator()(assembly::StackAssignment const&)
	{
		solAssert(false, "Assignment from stack is not supported in Yul.");
	}
	void operator()(assembly::Label const&)
	{
		solAssert(false, "Labels are not supported in Yul.");
	}
	void operator()(assembly::Literal const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::Identifier const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::VariableDeclaration const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::Assignment const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::FunctionDefinition const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::FunctionCall const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::Switch const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::ForLoop const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::Block const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
private:
	string m_assembly;
};

string yul::WebAssembly::assemble(assembly::Block const& _block)
{
	return Generator(_block).assembly();
}
