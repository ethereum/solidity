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

#include <libsolutil/Common.h>
#include <libsolutil/FixedHash.h>
#include <libsolutil/Whiskers.h>

#include <liblangutil/EVMVersion.h>

namespace solidity::yul::test::yul_fuzzer
{
class ProtoConverter
{
public:
	ProtoConverter(
		bool _filterStatefulInstructions = false,
		bool _filterUnboundedLoops = false
	)
	{
		m_funcVars = std::vector<std::vector<std::vector<std::string>>>{};
		m_globalVars = std::vector<std::vector<std::string>>{};
		m_inForBodyScope = false;
		m_inForInitScope = false;
		m_inForCond = false;
		m_numNestedForLoops = 0;
		m_numForLoops = 0;
		m_counter = 0;
		m_inputSize = 0;
		m_inFunctionDef = false;
		m_objectId = 0;
		m_isObject = false;
		m_forInitScopeExtEnabled = true;
		m_filterStatefulInstructions = _filterStatefulInstructions;
		m_filterUnboundedLoops = _filterUnboundedLoops;
	}
	ProtoConverter(ProtoConverter const&) = delete;
	ProtoConverter(ProtoConverter&&) = delete;
	std::string programToString(Program const& _input);

	/// Returns evm version
	solidity::langutil::EVMVersion version()
	{
		return m_evmVersion;
	}
private:
	void visit(BinaryOp const&);

	/// Visits a basic block optionally adding @a _funcParams to scope.
	/// @param _block Reference to a basic block of Yul statements.
	/// @param _funcParams List of function parameter names, defaults to
	/// an empty vector.
	void visit(Block const& _block);

	std::string visit(Literal const&);
	void visit(VarRef const&);
	void visit(Expression const&);
	void visit(VarDecl const&);
	void visit(MultiVarDecl const&);
	void visit(TypedVarDecl const&);
	void visit(UnaryOp const&);
	void visit(AssignmentStatement const&);
	void visit(IfStmt const&);
	void visit(StoreFunc const&);
	void visit(Statement const&);
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
	/// @param _f is the function call to be visited.
	/// @param _name is the name of the function called.
	/// @param _expression is a flag that is true if the function is called
	/// as a single-value expression, false otherwise.
	void visit(
		FunctionCall const& _f,
		std::string const& _name,
		bool _expression = false
	);
	void visit(FunctionDef const&);
	void visit(PopStmt const&);
	void visit(LeaveStmt const&);
	void visit(LowLevelCall const&);
	void visit(Create const&);
	void visit(UnaryOpData const&);
	void visit(Object const&);
	void visit(Data const&);
	void visit(Code const&);
	void visit(Program const&);

	/// Creates a new block scope.
	void openBlockScope();
	/// Creates a new function scope, and adds @a _funcParams to it if it
	/// is non-empty.
	void openFunctionScope(std::vector<std::string> const& _funcParams);
	/// Closes current block scope
	void closeBlockScope();
	/// Closes current function scope
	void closeFunctionScope();
	/// Adds @a _vars to current scope
	void addVarsToScope(std::vector<std::string> const& _vars);

	std::string createHex(std::string const& _hexBytes);

	/// Returns a new variable name.
	std::string newVarName()
	{
		return "x_" + std::to_string(counter());
	}

	/// Accepts an arbitrary string, removes all characters that are neither
	/// alphabets nor digits from it and returns the said string.
	static std::string createAlphaNum(std::string const& _strBytes);

	enum class NumFunctionReturns: unsigned
	{
		None = 0,
		Single,
		Multiple
	};

	void visitFunctionInputParams(FunctionCall const&, unsigned);
	void createFunctionDefAndCall(FunctionDef const&, unsigned, unsigned);

	/// Convert function type to a string to be used while naming a
	/// function that is created by a function declaration statement.
	/// @param _type Type classified according to the number of
	/// values returned by function.
	/// @return A string as follows. If _type is
	/// None -> "n"
	/// Single -> "s"
	/// Multiple -> "m"
	static std::string functionTypeToString(NumFunctionReturns _type);

	/// Builds a single vector containing variables declared in
	/// function scope.
	void consolidateVarDeclsInFunctionDef();

	/// Builds a single vector containing variables declared in
	/// global scope.
	void consolidateGlobalVarDecls();

	/// Return true if at least one variable declaration is in scope,
	/// false otherwise.
	/// @return True in the following cases:
	/// - If we are inside a function that has already declared a variable
	/// - If there is at least one variable declaration that is
	/// in scope
	bool varDeclAvailable();

	/// Converts protobuf function call to a Yul function call and appends
	/// it to output stream.
	/// @param _x Protobuf function call
	/// @param _name Function name
	/// @param _numInParams Number of input arguments accepted by function
	/// @param _newLine Flag that prints a new line to the output stream if
	/// true. Default value for the flag is true.
	void convertFunctionCall(
		FunctionCall const& _x,
		std::string const& _name,
		unsigned _numInParams,
		bool _newLine = true
	);

	/// Prints a Yul formatted variable declaration statement to the output
	/// stream.
	/// Example 1: createVarDecls(0, 1, true) returns {"x_0"} and prints
	///		let x_0 :=
	/// Example 2: createVarDecls(0, 2, false) returns {"x_0", "x_1"} and prints
	///		let x_0, x_1
	/// @param _start Start index of variable (inclusive)
	/// @param _end End index of variable (exclusive)
	/// @param _isAssignment Flag indicating if variable declaration is also
	/// an assignment. If true, the string " := " follows the variable
	/// declaration. Otherwise, a new line is follows the variable
	/// declaration.
	/// @return A vector of strings containing the variable names used in
	/// the declaration statement.
	std::vector<std::string> createVarDecls(unsigned _start, unsigned _end, bool _isAssignment);

	/// Prints comma separated variable names to output stream and
	/// returns a vector containing the printed variable names.
	/// Example: createVars(0, 2) returns {"x_0", "x_1"} and prints
	///		x_0, x_1
	/// @param _startIdx Start index of variable (inclusive)
	/// @param _endIdx End index of variable (exclusive)
	/// @return A vector of strings containing the printed variable names.
	std::vector<std::string> createVars(unsigned _startIdx, unsigned _endIdx);

	/// Manages scope of Yul variables
	/// @param _varNames is a list of Yul variable names whose scope needs
	/// to be tracked according to Yul scoping rules.
	void scopeVariables(std::vector<std::string> const& _varNames);

	/// Print the Yul syntax to make a call to a function named @a _funcName to
	/// the output stream.
	/// @param _funcName Name of the function to be called
	/// @param _numInParams Number of input parameters in function signature
	/// @param _numOutParams Number of output parameters in function signature
	void createFunctionCall(std::string const& _funcName, unsigned _numInParams, unsigned _numOutParams);

	/// Print the Yul syntax to pass input arguments to a function that has
	/// @a _numInParams number of input parameters to the output stream.
	/// The input arguments are pseudo-randomly chosen from calldata, memory,
	/// storage, or the Yul optimizer hex dictionary.
	/// @param _numInParams Number of input arguments to fill
	void fillFunctionCallInput(unsigned _numInParams);

	/// Print the Yul syntax to save values returned by a function call
	/// to the output stream. The values are either stored to memory or
	/// storage based on a simulated coin flip. The saved location is
	/// decided pseudo-randomly.
	/// @param _varsVec A vector of strings that reference variables
	/// holding the return values of a function call.
	void saveFunctionCallOutput(std::vector<std::string> const& _varsVec);

	/// Register a function declaration
	/// @param _f Pointer to a FunctionDef object
	void registerFunction(FunctionDef const* _f);

	/// Removes entry from m_functionMap and m_functionName
	void updateFunctionMaps(std::string const& _x);

	/// Build a tree of objects that contains the object/data
	/// identifiers that are in scope in a given object.
	/// @param _x root object of the Yul protobuf specification.
	void buildObjectScopeTree(Object const& _x);

	/// Returns a pseudo-random dictionary token.
	/// @param _p Enum that decides if the returned token is hex prefixed ("0x") or not
	/// @return Dictionary token at the index computed using a
	/// monotonically increasing counter as follows:
	///		index = (m_inputSize * m_inputSize + counter) % dictionarySize
	/// where m_inputSize is the size of the protobuf input and
	/// dictionarySize is the total number of entries in the dictionary.
	std::string dictionaryToken(util::HexPrefix _p = util::HexPrefix::Add);

	/// Returns an EVMVersion object corresponding to the protobuf
	/// enum of type Program_Version
	static solidity::langutil::EVMVersion evmVersionMapping(Program_Version const& _x);

	/// @returns name of Yul function with return type of @param _numReturns.
	std::optional<std::string> functionExists(NumFunctionReturns _numReturns);

	/// Returns a monotonically increasing counter that starts from zero.
	unsigned counter()
	{
		return m_counter++;
	}

	/// Generate function name of the form "foo_<typeSuffix>_<counter>".
	/// @param _type Type classified according to the number of
	/// values returned by function.
	std::string functionName(NumFunctionReturns _type)
	{
		return "foo_" + functionTypeToString(_type) + "_" + std::to_string(counter());
	}

	/// Returns a pseudo-randomly chosen object identifier that is in the
	/// scope of the Yul object being visited.
	std::string getObjectIdentifier(unsigned _x);

	/// Return new object identifier as string. Identifier string
	/// is a template of the form "\"object<n>\"" where <n> is
	/// a monotonically increasing object ID counter.
	/// @param _decorate If true (default value), object ID is
	/// enclosed within double quotes.
	std::string newObjectId(bool _decorate = true)
	{
		return util::Whiskers(R"(<?decorate>"</decorate>object<id><?decorate>"</decorate>)")
			("decorate", _decorate)
			("id", std::to_string(m_objectId++))
			.render();
	}

	/// Returns the object counter value corresponding to the object
	/// being visited.
	unsigned currentObjectId() const
	{
		return m_objectId - 1;
	}

	std::ostringstream m_output;
	/// Variables in all function definitions
	std::vector<std::vector<std::vector<std::string>>> m_funcVars;
	/// Variables in current function definition
	std::vector<std::string const*> m_currentFuncVars;
	/// Variables in global scope
	std::vector<std::string const*> m_currentGlobalVars;
	/// Functions in current scope
	std::vector<std::vector<std::string>> m_scopeFuncs;
	/// Global variables
	std::vector<std::vector<std::string>> m_globalVars;
	/// Variables declared in for loop init block that is in global scope
	std::vector<std::vector<std::string>> m_globalForLoopInitVars;
	/// Variables declared in for loop init block that is in function scope
	std::vector<std::vector<std::vector<std::string>>> m_funcForLoopInitVars;
	/// Vector of function names
	std::vector<std::string> m_functions;
	/// Maps FunctionDef object to its name
	std::map<FunctionDef const*, std::string> m_functionDefMap;
	// Set that is used for deduplicating switch case literals
	std::stack<std::set<u256>> m_switchLiteralSetPerScope;
	// Look-up table per function type that holds the number of input (output) function parameters
	std::map<std::string, std::pair<unsigned, unsigned>> m_functionSigMap;
	/// Map of object name to list of sub-object namespace(s) in scope
	std::map<std::string, std::vector<std::string>> m_objectScope;
	// mod input/output parameters impose an upper bound on the number of input/output parameters a function may have.
	static unsigned constexpr s_modInputParams = 5;
	static unsigned constexpr s_modOutputParams = 5;
	/// Hard-coded identifier for a Yul object's data block
	static auto constexpr s_dataIdentifier = "datablock";
	/// Predicate to keep track of for body scope. If false, break/continue
	/// statements can not be created.
	bool m_inForBodyScope;
	/// Maximum number of for loops that a test case may contain
	static auto constexpr s_maxForLoops = 2;
	// Index used for naming loop variable of bounded for loops
	unsigned m_numNestedForLoops;
	/// Counter for number of for loops
	unsigned m_numForLoops;
	/// Predicate to keep track of for loop init scope. If true, variable
	/// or function declarations can not be created.
	bool m_inForInitScope;
	/// Flag that is true while converting for loop condition,
	/// false otherwise.
	bool m_inForCond;
	/// Monotonically increasing counter
	unsigned m_counter;
	/// Size of protobuf input
	unsigned m_inputSize;
	/// Predicate that is true if inside function definition, false otherwise
	bool m_inFunctionDef;
	/// Index used for naming objects
	unsigned m_objectId;
	/// Flag to track whether program is an object (true) or a statement block
	/// (false: default value)
	bool m_isObject;
	/// Flag to track whether scope extension of variables defined in for-init
	/// block is enabled.
	bool m_forInitScopeExtEnabled;
	/// Object that holds the targeted evm version specified by protobuf input
	solidity::langutil::EVMVersion m_evmVersion;
	/// Flag that, if set, stops the converter from generating state changing
	/// opcodes.
	bool m_filterStatefulInstructions;
	/// Flat that, if set, stops the converter from generating potentially
	/// unbounded loops.
	bool m_filterUnboundedLoops;
};
}
