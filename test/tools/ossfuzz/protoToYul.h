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

#include <cstdint>
#include <cstddef>
#include <string>
#include <ostream>
#include <sstream>
#include <stack>
#include <set>
#include <vector>
#include <tuple>

#include <test/tools/ossfuzz/yulProto.pb.h>
#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>

namespace yul
{
namespace test
{
namespace yul_fuzzer
{
class ProtoConverter
{
public:
	ProtoConverter()
	{
		m_numFunctionSets = 0;
		m_inForBodyScope = false;
		m_inForInitScope = false;
		m_numNestedForLoops = 0;
		m_counter = 0;
		m_inputSize = 0;
	}
	ProtoConverter(ProtoConverter const&) = delete;
	ProtoConverter(ProtoConverter&&) = delete;
	std::string programToString(Program const& _input);

private:
	void visit(BinaryOp const&);
	/// Visits a basic block optionally adding @a _funcParams to scope.
	/// @param _block Reference to a basic block of yul statements.
	/// @param _funcParams List of function parameter names, defaults to
	/// an empty vector.
	void visit(Block const& _block, std::vector<std::string> _funcParams = {});

	/// Visits a basic block that contains a variable declaration at the
	/// very beginning, optionally adding @a _funcParams to scope.
	/// @param _block Reference to a basic block of yul statements.
	/// @param _funcParams List of function parameter names, defaults to
	/// an empty vector.
	void visit(SpecialBlock const& _block, std::vector<std::string> _funcParams = {});
	std::string visit(Literal const&);
	void visit(VarRef const&);
	void visit(Expression const&);
	void visit(VarDecl const&);
	void visit(EmptyVarDecl const&);
	void visit(MultiVarDecl const&);
	void visit(TypedVarDecl const&);
	void visit(UnaryOp const&);
	void visit(AssignmentStatement const&);
	void visit(MultiAssignment const&);
	void visit(IfStmt const&);
	void visit(StoreFunc const&);
	void visit(Statement const&);
	void visit(FunctionDefinition const&);
	void visit(ForStmt const&);
	void visit(BoundedForStmt const&);
	void visit(CaseStmt const&);
	void visit(SwitchStmt const&);
	void visit(TernaryOp const&);
	void visit(NullaryOp const&);
	void visit(LogFunc const&);
	void visit(CopyFunc const&);
	void visit(ExtCodeCopy const&);
	void visit(StopInvalidStmt const&);
	void visit(RetRevStmt const&);
	void visit(SelfDestructStmt const&);
	void visit(TerminatingStmt const&);
	void visit(FunctionCallNoReturnVal const&);
	void visit(FunctionCallSingleReturnVal const&);
	void visit(FunctionCall const&);
	void visit(FunctionDefinitionNoReturnVal const&);
	void visit(FunctionDefinitionSingleReturnVal const&);
	void visit(FunctionDefinitionMultiReturnVal const&);
	void visit(Program const&);
	void registerFunction(FunctionDefinition const&);

	/// Creates a new scope, and optionally adds @a _funcParams to it
	void openScope(std::vector<std::string> const& _funcParams);
	/// Closes current scope
	void closeScope();
	/// Adds @a _vars to current scope
	void addToScope(std::vector<std::string> const& _vars);

	std::string createHex(std::string const& _hexBytes);

	/// Returns a new variable name.
	std::string newVarName()
	{
		return "x_" + std::to_string(counter());
	}

	/// Accepts an arbitrary string, removes all characters that are neither
	/// alphabets nor digits from it and returns the said string.
	std::string createAlphaNum(std::string const& _strBytes);
	enum class NumFunctionReturns
	{
		None,
		Single,
		Multiple
	};

	template<class T>
	void visitFunctionInputParams(T const&, unsigned);

	template<class T>
	void createFunctionDefAndCall(T const&, unsigned, unsigned, NumFunctionReturns);
	std::string functionTypeToString(NumFunctionReturns _type);

	/// Creates variable declarations "x_<_startIdx>",...,"x_<_endIdx - 1>"
	std::vector<std::string> createVars(unsigned _startIdx, unsigned _endIdx);

	template <class T>
	void registerFunction(T const& _x, NumFunctionReturns _type, unsigned _numOutputParams = 0)
	{
		unsigned numInputParams = _x.num_input_params() % s_modInputParams;
		switch (_type)
		{
			case NumFunctionReturns::None:
				m_functionVecNoReturnValue.push_back(numInputParams);
				break;
			case NumFunctionReturns::Single:
				m_functionVecSingleReturnValue.push_back(numInputParams);
				break;
			case NumFunctionReturns::Multiple:
				m_functionVecMultiReturnValue.push_back(std::make_pair(numInputParams, _numOutputParams));
				break;
		}
	}
	/// Returns a pseudo-random dictionary token.
	/// @param _p Enum that decides if the returned token is hex prefixed ("0x") or not
	/// @return Dictionary token at the index computed using a
	/// monotonically increasing counter as follows:
	///		index = (m_inputSize * m_inputSize + counter) % dictionarySize
	/// where m_inputSize is the size of the protobuf input and
	/// dictionarySize is the total number of entries in the dictionary.
	std::string dictionaryToken(dev::HexPrefix _p = dev::HexPrefix::Add);

	/// Returns a monotonically increasing counter that starts from zero.
	unsigned counter()
	{
		return m_counter++;
	}

	std::ostringstream m_output;
	/// Scope
	std::stack<std::set<std::string>> m_scopes;
	/// Variables
	std::vector<std::string> m_variables;
	// Set that is used for deduplicating switch case literals
	std::stack<std::set<dev::u256>> m_switchLiteralSetPerScope;
	// Total number of function sets. A function set contains one function of each type defined by
	// NumFunctionReturns
	unsigned m_numFunctionSets;
	// Look-up table per function type that holds the number of input (output) function parameters
	std::vector<unsigned> m_functionVecNoReturnValue;
	std::vector<unsigned> m_functionVecSingleReturnValue;
	std::vector<std::pair<unsigned, unsigned>> m_functionVecMultiReturnValue;
	// mod input/output parameters impose an upper bound on the number of input/output parameters a function may have.
	static unsigned constexpr s_modInputParams = 5;
	static unsigned constexpr s_modOutputParams = 5;
	// predicate to keep track of for body scope
	bool m_inForBodyScope;
	// Index used for naming loop variable of bounded for loops
	unsigned m_numNestedForLoops;
	// predicate to keep track of for loop init scope
	bool m_inForInitScope;
	/// Monotonically increasing counter
	unsigned m_counter;
	/// Size of protobuf input
	unsigned m_inputSize;
};
}
}
}
