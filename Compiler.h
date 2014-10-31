/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity AST to EVM bytecode compiler.
 */

#include <ostream>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/CompilerContext.h>

namespace dev {
namespace solidity {

class Compiler: private ASTVisitor
{
public:
	Compiler(): m_returnTag(m_context.newTag()) {}

	void compileContract(ContractDefinition& _contract);
	bytes getAssembledBytecode() { return m_context.getAssembledBytecode(); }
	void streamAssembly(std::ostream& _stream) const { m_context.streamAssembly(_stream); }

	/// Compile the given contract and return the EVM bytecode.
	static bytes compile(ContractDefinition& _contract);

private:
	/// Creates a new compiler context / assembly and packs the current code into the data part.
	void packIntoContractCreator();
	void appendFunctionSelector(std::vector<ASTPointer<FunctionDefinition> > const& _functions);
	void appendCalldataUnpacker(FunctionDefinition const& _function);
	void appendReturnValuePacker(FunctionDefinition const& _function);

	virtual bool visit(FunctionDefinition& _function) override;
	virtual bool visit(IfStatement& _ifStatement) override;
	virtual bool visit(WhileStatement& _whileStatement) override;
	virtual bool visit(Continue& _continue) override;
	virtual bool visit(Break& _break) override;
	virtual bool visit(Return& _return) override;
	virtual bool visit(VariableDefinition& _variableDefinition) override;
	virtual bool visit(ExpressionStatement& _expressionStatement) override;


	CompilerContext m_context;
	std::vector<eth::AssemblyItem> m_breakTags; ///< tag to jump to for a "break" statement
	std::vector<eth::AssemblyItem> m_continueTags; ///< tag to jump to for a "continue" statement
	eth::AssemblyItem m_returnTag; ///< tag to jump to for a "return" statement
};

}
}
