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
 * Common code generator for translating Julia / inline assembly to EVM and EVM1.5.
 */

#include <libjulia/backends/evm/AbstractAssembly.h>

#include <libsolidity/inlineasm/AsmStack.h>
#include <libsolidity/inlineasm/AsmScope.h>

#include <boost/variant.hpp>

namespace dev
{
namespace solidity
{
class ErrorReporter;
namespace assembly
{
struct Literal;
struct Block;
struct Switch;
struct Label;
struct FunctionalInstruction;
struct Assignment;
struct VariableDeclaration;
struct Instruction;
struct Identifier;
struct StackAssignment;
struct FunctionDefinition;
struct FunctionCall;

struct AsmAnalysisInfo;
}
}
namespace julia
{

class CodeTransform: public boost::static_visitor<>
{
public:
	/// Create the code transformer which appends assembly to _assembly as a side-effect
	/// of its creation.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	CodeTransform(
		solidity::ErrorReporter& _errorReporter,
		julia::AbstractAssembly& _assembly,
		solidity::assembly::Block const& _block,
		solidity::assembly::AsmAnalysisInfo& _analysisInfo,
		ExternalIdentifierAccess const& _identifierAccess = ExternalIdentifierAccess()
	): CodeTransform(_errorReporter, _assembly, _block, _analysisInfo, _identifierAccess, _assembly.stackHeight())
	{
	}

private:
	CodeTransform(
		solidity::ErrorReporter& _errorReporter,
		julia::AbstractAssembly& _assembly,
		solidity::assembly::Block const& _block,
		solidity::assembly::AsmAnalysisInfo& _analysisInfo,
		ExternalIdentifierAccess const& _identifierAccess,
		int _initialStackHeight
	);

public:
	void operator()(solidity::assembly::Instruction const& _instruction);
	void operator()(solidity::assembly::Literal const& _literal);
	void operator()(solidity::assembly::Identifier const& _identifier);
	void operator()(solidity::assembly::FunctionalInstruction const& _instr);
	void operator()(solidity::assembly::FunctionCall const&);
	void operator()(solidity::assembly::Label const& _label);
	void operator()(solidity::assembly::StackAssignment const& _assignment);
	void operator()(solidity::assembly::Assignment const& _assignment);
	void operator()(solidity::assembly::VariableDeclaration const& _varDecl);
	void operator()(solidity::assembly::Block const& _block);
	void operator()(solidity::assembly::Switch const& _switch);
	void operator()(solidity::assembly::FunctionDefinition const&);

private:
	void generateAssignment(solidity::assembly::Identifier const& _variableName, SourceLocation const& _location);

	/// Determines the stack height difference to the given variables. Automatically generates
	/// errors if it is not yet in scope or the height difference is too large. Returns 0 on
	/// errors and the (positive) stack height difference otherwise.
	int variableHeightDiff(solidity::assembly::Scope::Variable const& _var, SourceLocation const& _location, bool _forSwap);

	void expectDeposit(int _deposit, int _oldHeight);

	void checkStackHeight(void const* _astElement);

	/// Assigns the label's id to a value taken from eth::Assembly if it has not yet been set.
	void assignLabelIdIfUnset(solidity::assembly::Scope::Label& _label);

	solidity::ErrorReporter& m_errorReporter;
	julia::AbstractAssembly& m_assembly;
	solidity::assembly::AsmAnalysisInfo& m_info;
	solidity::assembly::Scope& m_scope;
	ExternalIdentifierAccess m_identifierAccess;
	int const m_initialStackHeight;
};

}
}
