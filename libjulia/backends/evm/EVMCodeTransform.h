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

#include <libjulia/backends/evm/EVMAssembly.h>

#include <libsolidity/inlineasm/AsmScope.h>
#include <libsolidity/inlineasm/AsmDataForward.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

namespace dev
{
namespace solidity
{
class ErrorReporter;
namespace assembly
{
struct AsmAnalysisInfo;
}
}
namespace julia
{
class EVMAssembly;

class CodeTransform: public boost::static_visitor<>
{
public:
	/// Create the code transformer.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	CodeTransform(
		julia::AbstractAssembly& _assembly,
		solidity::assembly::AsmAnalysisInfo& _analysisInfo,
		bool _julia = false,
		bool _evm15 = false,
		ExternalIdentifierAccess const& _identifierAccess = ExternalIdentifierAccess(),
		bool _useNamedLabelsForFunctions = false
	): CodeTransform(
		_assembly,
		_analysisInfo,
		_julia,
		_evm15,
		_identifierAccess,
		_useNamedLabelsForFunctions,
		_assembly.stackHeight(),
		std::make_shared<Context>()
	)
	{
	}

protected:
	struct Context
	{
		using Scope = solidity::assembly::Scope;
		std::map<Scope::Label const*, AbstractAssembly::LabelID> labelIDs;
		std::map<Scope::Function const*, AbstractAssembly::LabelID> functionEntryIDs;
		std::map<Scope::Variable const*, int> variableStackHeights;
	};

	CodeTransform(
		julia::AbstractAssembly& _assembly,
		solidity::assembly::AsmAnalysisInfo& _analysisInfo,
		bool _julia,
		bool _evm15,
		ExternalIdentifierAccess const& _identifierAccess,
		bool _useNamedLabelsForFunctions,
		int _stackAdjustment,
		std::shared_ptr<Context> _context
	):
		m_assembly(_assembly),
		m_info(_analysisInfo),
		m_julia(_julia),
		m_evm15(_evm15),
		m_useNamedLabelsForFunctions(_useNamedLabelsForFunctions),
		m_identifierAccess(_identifierAccess),
		m_stackAdjustment(_stackAdjustment),
		m_context(_context)
	{}

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
	void operator()(solidity::assembly::Switch const& _switch);
	void operator()(solidity::assembly::FunctionDefinition const&);
	void operator()(solidity::assembly::ForLoop const&);
	void operator()(solidity::assembly::Block const& _block);

private:
	AbstractAssembly::LabelID labelFromIdentifier(solidity::assembly::Identifier const& _identifier);
	/// @returns the label ID corresponding to the given label, allocating a new one if
	/// necessary.
	AbstractAssembly::LabelID labelID(solidity::assembly::Scope::Label const& _label);
	AbstractAssembly::LabelID functionEntryID(std::string const& _name, solidity::assembly::Scope::Function const& _function);
	/// Generates code for an expression that is supposed to return a single value.
	void visitExpression(solidity::assembly::Statement const& _expression);

	void visitStatements(std::vector<solidity::assembly::Statement> const& _statements);

	/// Pops all variables declared in the block and checks that the stack height is equal
	/// to @a _blackStartStackHeight.
	void finalizeBlock(solidity::assembly::Block const& _block, int _blockStartStackHeight);

	void generateMultiAssignment(std::vector<solidity::assembly::Identifier> const& _variableNames);
	void generateAssignment(solidity::assembly::Identifier const& _variableName);

	/// Determines the stack height difference to the given variables. Throws
	/// if it is not yet in scope or the height difference is too large. Returns
	/// the (positive) stack height difference otherwise.
	int variableHeightDiff(solidity::assembly::Scope::Variable const& _var, bool _forSwap);

	void expectDeposit(int _deposit, int _oldHeight);

	void checkStackHeight(void const* _astElement);

	julia::AbstractAssembly& m_assembly;
	solidity::assembly::AsmAnalysisInfo& m_info;
	solidity::assembly::Scope* m_scope = nullptr;
	bool m_julia = false;
	bool m_evm15 = false;
	bool m_useNamedLabelsForFunctions = false;
	ExternalIdentifierAccess m_identifierAccess;
	/// Adjustment between the stack height as determined during the analysis phase
	/// and the stack height in the assembly. This is caused by an initial stack being present
	/// for inline assembly and different stack heights depending on the EVM backend used
	/// (EVM 1.0 or 1.5).
	int m_stackAdjustment = 0;
	std::shared_ptr<Context> m_context;
};

}
}
