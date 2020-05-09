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

#include <test/tools/ossfuzz/protoToYul.h>
#include <test/tools/ossfuzz/yulOptimizerFuzzDictionary.h>

#include <libyul/Exceptions.h>

#include <libsolutil/StringUtils.h>

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace solidity::yul::test::yul_fuzzer;
using namespace solidity::yul::test;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity;

vector<string> ProtoConverter::createVars(unsigned _startIdx, unsigned _endIdx)
{
	yulAssert(_endIdx > _startIdx, "Proto fuzzer: Variable indices not in range");
	string varsStr = suffixedVariableNameList("x_", _startIdx, _endIdx);
	m_output << varsStr;
	vector<string> varsVec;
	boost::split(
		varsVec,
		varsStr,
		boost::algorithm::is_any_of(", "),
		boost::algorithm::token_compress_on
	);

	yulAssert(
		varsVec.size() == (_endIdx - _startIdx),
		"Proto fuzzer: Variable count mismatch during function definition"
	);
	m_counter += varsVec.size();
	return varsVec;
}


string ProtoConverter::dummyExpression()
{
	string expression{};
	string location{};
	unsigned pseudoRandomNum = m_inputSize / 13;
	if (varDeclAvailable())
		location = varRef(pseudoRandomNum);
	switch (pseudoRandomNum % 4)
	{
	case 0:
		if (location.empty())
			expression = "mload(0)";
		else
			expression = Whiskers(R"(mload(mod(<loc>, 256)))")("loc", location).render();
		break;
	case 1:
		if (location.empty())
			expression = "sload(0)";
		else
			expression = Whiskers(R"(sload(mod(<loc>, 256)))")("loc", location).render();
		break;
	case 2:
		if (location.empty())
			expression = "calldataload(0)";
		else
			expression = Whiskers(R"(calldataload(mod(<loc>, 32)))")("loc", location).render();
		break;
	case 3:
		expression = dictionaryToken();
		break;
	}
	yulAssert(!expression.empty(), "Proto fuzzer: Invalid dummy expression");
	return expression;
}

string ProtoConverter::dictionaryToken(HexPrefix _p)
{
	std::string token;

       // If dictionary constant is requested while converting
       // for loop condition, then return zero so that we don't
       // generate infinite for loops.
       if (m_inForCond)
               token = "0";
       else
       {
               unsigned indexVar = m_inputSize * m_inputSize + counter();
               token = hexDictionary[indexVar % hexDictionary.size()];
               yulAssert(token.size() <= 64, "Proto Fuzzer: Dictionary token too large");
       }

	return _p == HexPrefix::Add ? "0x" + token : token;
}

string ProtoConverter::createHex(string const& _hexBytes)
{
	string tmp{_hexBytes};
	if (!tmp.empty())
	{
		boost::range::remove_erase_if(tmp, [=](char c) -> bool {
			return !std::isxdigit(c);
		});
		tmp = tmp.substr(0, 64);
	}
	// We need this awkward if case because hex literals cannot be empty.
	// Use a dictionary token.
	if (tmp.empty())
		tmp = dictionaryToken(HexPrefix::DontAdd);
	// Hex literals must have even number of digits
	if (tmp.size() % 2)
		tmp.insert(0, "0");

	yulAssert(tmp.size() <= 64, "Proto Fuzzer: Dictionary token too large");
	return tmp;
}

string ProtoConverter::createAlphaNum(string const& _strBytes)
{
	string tmp{_strBytes};
	if (!tmp.empty())
	{
		boost::range::remove_erase_if(tmp, [=](char c) -> bool {
			return !(std::isalpha(c) || std::isdigit(c));
		});
		tmp = tmp.substr(0, 32);
	}
	return tmp;
}

string ProtoConverter::visit(Literal const& _x)
{
	switch (_x.literal_oneof_case())
	{
	case Literal::kIntval:
		return to_string(_x.intval());
	case Literal::kHexval:
		return "0x" + createHex(_x.hexval());
	case Literal::kStrval:
		return "\"" + createAlphaNum(_x.strval()) + "\"";
	case Literal::kBoolval:
		return _x.boolval() ? "true" : "false";
	case Literal::LITERAL_ONEOF_NOT_SET:
		return dictionaryToken();
	}
}

void ProtoConverter::consolidateVarDeclsInFunctionDef()
{
	m_currentFuncVars.clear();
	yulAssert(!m_funcVars.empty(), "Proto fuzzer: Invalid operation");

	auto const& scopes = m_funcVars.back();
	for (auto const& s: scopes)
		for (auto const& var: s)
			m_currentFuncVars.push_back(&var);
	yulAssert(!m_funcForLoopInitVars.empty(), "Proto fuzzer: Invalid operation");
	auto const& forinitscopes = m_funcForLoopInitVars.back();
	for (auto const& s: forinitscopes)
		for (auto const& var: s)
			m_currentFuncVars.push_back(&var);
}

void ProtoConverter::consolidateGlobalVarDecls()
{
	m_currentGlobalVars.clear();
	// Place pointers to all global variables that are in scope
	// into a single vector
	for (auto const& scope: m_globalVars)
		for (auto const& var: scope)
			m_currentGlobalVars.push_back(&var);
	// Place pointers to all variables declared in for-init blocks
	// that are still live into the same vector
	for (auto const& init: m_globalForLoopInitVars)
		for (auto const& var: init)
			m_currentGlobalVars.push_back(&var);
}

bool ProtoConverter::varDeclAvailable()
{
	if (m_inFunctionDef)
	{
		consolidateVarDeclsInFunctionDef();
		return m_currentFuncVars.size() > 0;
	}
	else
	{
		consolidateGlobalVarDecls();
		return m_currentGlobalVars.size() > 0;
	}
}

string ProtoConverter::varRef(unsigned _index)
{
	if (m_inFunctionDef)
	{
		// Ensure that there is at least one variable declaration to reference in function scope.
		yulAssert(m_currentFuncVars.size() > 0, "Proto fuzzer: No variables to reference.");
		return *m_currentFuncVars[_index % m_currentFuncVars.size()];
	}
	else
	{
		// Ensure that there is at least one variable declaration to reference in nested scopes.
		yulAssert(m_currentGlobalVars.size() > 0, "Proto fuzzer: No global variables to reference.");
		return *m_currentGlobalVars[_index % m_currentGlobalVars.size()];
	}
}

void ProtoConverter::visit(VarRef const& _x)
{
	m_output << varRef(_x.varnum());
}

void ProtoConverter::visit(FunctionExpr const& _x)
{
	vector<pair<string, unsigned>> functionSet;
	for (auto const& f: m_functionSigMap)
		if (f.second.second == 1)
			functionSet.emplace_back(f.first, f.second.first);
	if (functionSet.size() > 0)
	{
		pair<string, unsigned> chosenFunction = functionSet[_x.index() % functionSet.size()];
		convertFunctionCall(_x, chosenFunction.first, chosenFunction.second);
	}
	else
	{
		m_output << dummyExpression();
	}
}

void ProtoConverter::visit(Expression const& _x)
{
	switch (_x.expr_oneof_case())
	{
	case Expression::kVarref:
		// If the expression requires a variable reference that we cannot provide
		// (because there are no variables in scope), we silently output a literal
		// expression from the optimizer dictionary.
		if (!varDeclAvailable())
			m_output << dummyExpression();
		else
			visit(_x.varref());
		break;
	case Expression::kCons:
               // If literal expression describes for-loop condition
               // then force it to zero, so we don't generate infinite
               // for loops
               if (m_inForCond)
                       m_output << "0";
               else
                       m_output << visit(_x.cons());
		break;
	case Expression::kBinop:
		visit(_x.binop());
		break;
	case Expression::kUnop:
		visit(_x.unop());
		break;
	case Expression::kTop:
		visit(_x.top());
		break;
	case Expression::kNop:
		visit(_x.nop());
		break;
	case Expression::kFuncexpr:
		visit(_x.funcexpr());
		break;
	case Expression::EXPR_ONEOF_NOT_SET:
		m_output << dummyExpression();
		break;
	}
}

void ProtoConverter::visit(BinaryOp const& _x)
{
	BinaryOp_BOp op = _x.op();

	switch (op)
	{
	case BinaryOp::ADD:
		m_output << "add";
		break;
	case BinaryOp::SUB:
		m_output << "sub";
		break;
	case BinaryOp::MUL:
		m_output << "mul";
		break;
	case BinaryOp::DIV:
		m_output << "div";
		break;
	case BinaryOp::MOD:
		m_output << "mod";
		break;
	case BinaryOp::XOR:
		m_output << "xor";
		break;
	case BinaryOp::AND:
		m_output << "and";
		break;
	case BinaryOp::OR:
		m_output << "or";
		break;
	case BinaryOp::EQ:
		m_output << "eq";
		break;
	case BinaryOp::LT:
		m_output << "lt";
		break;
	case BinaryOp::GT:
		m_output << "gt";
		break;
	case BinaryOp::SHR:
		m_output << "shr";
		break;
	case BinaryOp::SHL:
		m_output << "shl";
		break;
	case BinaryOp::SAR:
		m_output << "sar";
		break;
	case BinaryOp::SDIV:
		m_output << "sdiv";
		break;
	case BinaryOp::SMOD:
		m_output << "smod";
		break;
	case BinaryOp::EXP:
		m_output << "exp";
		break;
	case BinaryOp::SLT:
		m_output << "slt";
		break;
	case BinaryOp::SGT:
		m_output << "sgt";
		break;
	case BinaryOp::BYTE:
		m_output << "byte";
		break;
	case BinaryOp::SI:
		m_output << "signextend";
		break;
	case BinaryOp::KECCAK:
		m_output << "keccak256";
		break;
	}
	m_output << "(";
	visit(_x.left());
	m_output << ",";
	visit(_x.right());
	m_output << ")";
}

void ProtoConverter::scopeVariables(vector<string> const& _varNames)
{
	// If we are inside a for-init block, there are two places
	// where the visited vardecl may have been defined:
	// - directly inside the for-init block
	// - inside a block within the for-init block
	// In the latter case, we don't scope extend. The flag
	// m_forInitScopeExtEnabled (= true) indicates whether we are directly
	// inside a for-init block e.g., for { let x } or (= false) inside a
	// nested for-init block e.g., for { { let x } }
	bool forInitScopeExtendVariable = m_inForInitScope && m_forInitScopeExtEnabled;

	// There are four cases that are tackled here
	// Case 1. We are inside a function definition and the variable declaration's
	// scope needs to be extended.
	// Case 2. We are inside a function definition but scope extension is disabled
	// Case 3. We are inside global scope and scope extension is required
	// Case 4. We are inside global scope but scope extension is disabled
	if (m_inFunctionDef)
	{
		// Variables declared directly in for-init block
		// are tracked separately because their scope
		// extends beyond the block they are defined in
		// to the rest of the for-loop statement.
		// Case 1
		if (forInitScopeExtendVariable)
		{
			yulAssert(
				!m_funcForLoopInitVars.empty() && !m_funcForLoopInitVars.back().empty(),
				"Proto fuzzer: Invalid operation"
			);
			for (auto const& varName: _varNames)
				m_funcForLoopInitVars.back().back().push_back(varName);
		}
		// Case 2
		else
		{
			yulAssert(
				!m_funcVars.empty() && !m_funcVars.back().empty(),
				"Proto fuzzer: Invalid operation"
			);
			for (auto const& varName: _varNames)
				m_funcVars.back().back().push_back(varName);
		}
	}
	// If m_inFunctionDef is false, we are in global scope
	else
	{
		// Case 3
		if (forInitScopeExtendVariable)
		{
			yulAssert(!m_globalForLoopInitVars.empty(), "Proto fuzzer: Invalid operation");

			for (auto const& varName: _varNames)
				m_globalForLoopInitVars.back().push_back(varName);
		}
		// Case 4
		else
		{
			yulAssert(!m_globalVars.empty(), "Proto fuzzer: Invalid operation");

			for (auto const& varName: _varNames)
				m_globalVars.back().push_back(varName);
		}
	}
}

void ProtoConverter::visit(UnaryOp const& _x)
{
	UnaryOp_UOp op = _x.op();

	switch (op)
	{
	case UnaryOp::NOT:
		m_output << "not";
		break;
	case UnaryOp::MLOAD:
		m_output << "mload(mod";
		break;
	case UnaryOp::SLOAD:
		m_output << "sload(mod";
		break;
	case UnaryOp::ISZERO:
		m_output << "iszero";
		break;
	case UnaryOp::CALLDATALOAD:
		m_output << "calldataload(mod";
		break;
	}
	m_output << "(";
	visit(_x.operand());
	if (op == UnaryOp::MLOAD || op == UnaryOp::SLOAD || op == UnaryOp::CALLDATALOAD)
		m_output << ", 256)";
	m_output << ")";
}

void ProtoConverter::visit(TernaryOp const& _x)
{
	switch (_x.op())
	{
	case TernaryOp::ADDM:
		m_output << "addmod";
		break;
	case TernaryOp::MULM:
		m_output << "mulmod";
		break;
	}
	m_output << "(";
	visit(_x.arg1());
	m_output << ", ";
	visit(_x.arg2());
	m_output << ", ";
	visit(_x.arg3());
	m_output << ")";
}

void ProtoConverter::visit(NullaryOp const& _x)
{
	switch (_x.op())
	{
	case NullaryOp::CALLDATASIZE:
		m_output << "calldatasize()";
		break;
	case NullaryOp::RETURNDATASIZE:
		m_output << "returndatasize()";
		break;
	case NullaryOp::ADDRESS:
		m_output << "address()";
		break;
	case NullaryOp::ORIGIN:
		m_output << "origin()";
		break;
	case NullaryOp::CALLER:
		m_output << "caller()";
		break;
	case NullaryOp::CALLVALUE:
		m_output << "callvalue()";
		break;
	case NullaryOp::GASPRICE:
		m_output << "gasprice()";
		break;
	case NullaryOp::COINBASE:
		m_output << "coinbase()";
		break;
	case NullaryOp::NUMBER:
		m_output << "number()";
		break;
	case NullaryOp::DIFFICULTY:
		m_output << "difficulty()";
		break;
	case NullaryOp::GASLIMIT:
		m_output << "gaslimit()";
		break;
	case NullaryOp::SELFBALANCE:
		m_output << "selfbalance()";
		break;
	case NullaryOp::CHAINID:
		m_output << "chainid()";
		break;
	}
}

void ProtoConverter::visit(AssignmentStatement const& _x)
{
	visit(_x.ref_id());
	m_output << " := ";
	visit(_x.expr());
	m_output << "\n";
}

template <typename T>
void ProtoConverter::visitFunctionInputParams(T const& _x, unsigned _numInputParams)
{
	// We reverse the order of function input visits since it helps keep this switch case concise.
	switch (_numInputParams)
	{
	case 4:
		visit(_x.in_param4());
		m_output << ", ";
		[[fallthrough]];;
	case 3:
		visit(_x.in_param3());
		m_output << ", ";
		[[fallthrough]];;
	case 2:
		visit(_x.in_param2());
		m_output << ", ";
		[[fallthrough]];;
	case 1:
		visit(_x.in_param1());
		[[fallthrough]];;
	case 0:
		break;
	default:
		yulAssert(false, "Proto fuzzer: Function call with too many input parameters.");
		break;
	}
}

template <typename T>
void ProtoConverter::convertFunctionCall(
	T const& _x,
	std::string _name,
	unsigned _numInParams,
	bool _newline
)
{
	m_output << _name << "(";
	visitFunctionInputParams(_x, _numInParams);
	m_output << ")";
	if (_newline)
		m_output << "\n";
}

void ProtoConverter::visit(FunctionCall const& _x)
{
	bool functionAvailable = m_functionSigMap.size() > 0;
	unsigned numInParams, numOutParams;
	string funcName;
	if (functionAvailable)
	{
		yulAssert(m_functions.size() > 0, "Proto fuzzer: No function in scope");
		funcName = m_functions[_x.func_index() % m_functions.size()];
		auto ret = m_functionSigMap.at(funcName);
		numInParams = ret.first;
		numOutParams = ret.second;
	}
	else
	{
		return;
	}

	// If we are here, it means that we have at least one valid
	// function for making the function call
	switch (numOutParams)
	{
	case 0:
		convertFunctionCall(_x, funcName, numInParams, /*newline=*/true);
		break;
	case 1:
		// assignment
		if (varDeclAvailable())
		{
			visit(_x.out_param1());
			m_output << " := ";
			convertFunctionCall(_x, funcName, numInParams, /*newline=*/true);
		}
		break;
	case 2:
	[[fallthrough]];
	case 3:
	[[fallthrough]];
	case 4:
	{
		// Ensure that the chosen function returns at most 4 values
		yulAssert(
			numOutParams <= 4,
			"Proto fuzzer: Function call with too many output params encountered."
		);
		{
			// If we do not have variables to assign to, bail out
			if (!varDeclAvailable())
				return;

			// Convert LHS of multi assignment
			// We reverse the order of out param visits since the order does not matter.
			// This helps reduce the size of this switch statement.
			switch (numOutParams)
			{
			case 4:
				visit(_x.out_param4());
				m_output << ", ";
				[[fallthrough]];
			case 3:
				visit(_x.out_param3());
				m_output << ", ";
				[[fallthrough]];
			case 2:
				visit(_x.out_param2());
				m_output << ", ";
				visit(_x.out_param1());
				break;
			default:
				yulAssert(false, "Proto fuzzer: Function call with too many or too few input parameters.");
				break;
			}
			m_output << " := ";

			// Convert RHS of multi assignment
			convertFunctionCall(_x, funcName, numInParams, /*newline=*/true);
		}
		break;
	}
	}
}

void ProtoConverter::visit(IfStmt const& _x)
{
	m_output << "if ";
	visit(_x.cond());
	m_output << " ";
	visit(_x.block());
}

void ProtoConverter::visit(StoreFunc const& _x)
{
	switch (_x.st())
	{
	case StoreFunc::MSTORE:
		m_output << "mstore(mod(";
		break;
	case StoreFunc::SSTORE:
		m_output << "sstore(mod(";
		break;
	case StoreFunc::MSTORE8:
		m_output << "mstore8(mod(";
		break;
	}
	visit(_x.loc());
	m_output << ", 256), ";
	visit(_x.val());
	m_output << ")\n";
}

void ProtoConverter::visit(ForStmt const& _x)
{
	if (++m_numForLoops > s_maxForLoops)
		return;
	bool wasInForBody = m_inForBodyScope;
	bool wasInForInit = m_inForInitScope;
	bool wasForInitScopeExtEnabled = m_forInitScopeExtEnabled;
	m_inForBodyScope = false;
	m_inForInitScope = true;
	m_forInitScopeExtEnabled = true;
	m_inForCond = false;
	m_output << "for ";
	visit(_x.for_init());
	m_inForInitScope = false;
	m_forInitScopeExtEnabled = wasForInitScopeExtEnabled;
	m_inForCond = true;
	visit(_x.for_cond());
	m_inForCond = false;
	visit(_x.for_post());
	m_inForBodyScope = true;
	visit(_x.block());
	m_inForBodyScope = wasInForBody;
	m_inForInitScope = wasInForInit;
	if (m_inFunctionDef)
	{
		yulAssert(
			!m_funcForLoopInitVars.empty() && !m_funcForLoopInitVars.back().empty(),
			"Proto fuzzer: Invalid data structure"
		);
		// Remove variables in for-init
		m_funcForLoopInitVars.back().pop_back();
	}
	else
	{
		yulAssert(!m_globalForLoopInitVars.empty(), "Proto fuzzer: Invalid data structure");
		m_globalForLoopInitVars.pop_back();
	}
}

void ProtoConverter::visit(BoundedForStmt const& _x)
{
	if (++m_numForLoops > s_maxForLoops)
		return;

	// Boilerplate for loop that limits the number of iterations to a maximum of 4.
	std::string loopVarName("i_" + std::to_string(m_numNestedForLoops++));
	m_output << "for { let " << loopVarName << " := 0 } "
	       << "lt(" << loopVarName << ", 0x60) "
	       << "{ " << loopVarName << " := add(" << loopVarName << ", 0x20) } ";
	// Store previous for body scope
	bool wasInForBody = m_inForBodyScope;
	bool wasInForInit = m_inForInitScope;
	m_inForBodyScope = true;
	m_inForInitScope = false;
	visit(_x.block());
	// Restore previous for body scope and init
	m_inForBodyScope = wasInForBody;
	m_inForInitScope = wasInForInit;
}

void ProtoConverter::visit(CaseStmt const& _x)
{
	string literal = visit(_x.case_lit());
	// u256 value of literal
	u256 literalVal;

	// Convert string to u256 before looking for duplicate case literals
	if (_x.case_lit().has_strval())
	{
		// Since string literals returned by the Literal visitor are enclosed within
		// double quotes (like this "\"<string>\""), their size is at least two in the worst case
		// that <string> is empty. Here we assert this invariant.
		yulAssert(literal.size() >= 2, "Proto fuzzer: String literal too short");
		// This variable stores the <string> part i.e., literal minus the first and last
		// double quote characters. This is used to compute the keccak256 hash of the
		// string literal. The hashing is done to check whether we are about to create
		// a case statement containing a case literal that has already been used in a
		// previous case statement. If the hash (u256 value) matches a previous hash,
		// then we simply don't create a new case statement.
		string noDoubleQuoteStr{""};
		if (literal.size() > 2)
		{
			// Ensure that all characters in the string literal except the first
			// and the last (double quote characters) are alphanumeric.
			yulAssert(
				boost::algorithm::all_of(literal.begin() + 1, literal.end() - 2, [=](char c) -> bool {
					return std::isalpha(c) || std::isdigit(c);
				}),
				"Proto fuzzer: Invalid string literal encountered"
			);

			// Make a copy because literal will need to be used later
			noDoubleQuoteStr = literal.substr(1, literal.size() - 2);
		}
		// Hash the result to check for duplicate case literal strings
		literalVal = u256(h256(noDoubleQuoteStr, h256::FromBinary, h256::AlignLeft));

		// Make sure that an empty string literal evaluates to zero. This is to detect creation of
		// duplicate case literals like so
		// switch (x)
		// {
		//    case "": { x := 0 }
		//    case 0: { x:= 1 } // Case statement with duplicate literal is invalid
		// } // This snippet will not be parsed successfully.
		if (noDoubleQuoteStr.empty())
			yulAssert(literalVal == 0, "Proto fuzzer: Empty string does not evaluate to zero");
	}
	else if (_x.case_lit().has_boolval())
		literalVal = _x.case_lit().boolval() ? u256(1) : u256(0);
	else
		literalVal = u256(literal);

	// Check if set insertion fails (case literal present) or succeeds (case literal
	// absent).
	bool isUnique = m_switchLiteralSetPerScope.top().insert(literalVal).second;

	// It is fine to bail out if we encounter a duplicate case literal because
	// we can be assured that the switch statement is well-formed i.e., contains
	// at least one case statement or a default block.
	if (isUnique)
	{
		m_output << "case " << literal << " ";
		visit(_x.block());
	}
}

void ProtoConverter::visit(SwitchStmt const& _x)
{
	if (_x.case_stmt_size() > 0 || _x.has_block())
	{
		std::set<u256> s;
		m_switchLiteralSetPerScope.push(s);
		m_output << "switch ";
		visit(_x.switch_expr());
		m_output << "\n";

		for (auto const& caseStmt: _x.case_stmt())
			visit(caseStmt);

		m_switchLiteralSetPerScope.pop();

		if (_x.has_block())
		{
			m_output << "default ";
			visit(_x.block());
		}
	}
}

void ProtoConverter::visit(Statement const& _x)
{
	switch (_x.stmt_oneof_case())
	{
	case Statement::kAssignment:
		// Create an assignment statement only if there is at least one variable
		// declaration that is in scope.
		if (varDeclAvailable())
			visit(_x.assignment());
		break;
	case Statement::kIfstmt:
		if (_x.ifstmt().block().statements_size() > 0)
			visit(_x.ifstmt());
		break;
	case Statement::kStorageFunc:
		visit(_x.storage_func());
		break;
	case Statement::kBlockstmt:
		if (_x.blockstmt().statements_size() > 0)
			visit(_x.blockstmt());
		break;
	case Statement::kForstmt:
		if (_x.forstmt().block().statements_size() > 0)
			visit(_x.forstmt());
		break;
	case Statement::kBoundedforstmt:
		if (_x.boundedforstmt().block().statements_size() > 0)
			visit(_x.boundedforstmt());
		break;
	case Statement::kSwitchstmt:
		visit(_x.switchstmt());
		break;
	case Statement::kBreakstmt:
		if (m_inForBodyScope)
			m_output << "break\n";
		break;
	case Statement::kContstmt:
		if (m_inForBodyScope)
			m_output << "continue\n";
		break;
	case Statement::kFunctioncall:
		visit(_x.functioncall());
		break;
	case Statement::kFuncdef:
		if (_x.funcdef().block().statements_size() > 0)
			if (!m_inForInitScope)
				visit(_x.funcdef());
		break;
	case Statement::kPop:
		visit(_x.pop());
		break;
	case Statement::kLeave:
		if (m_inFunctionDef)
			visit(_x.leave());
		break;
	case Statement::STMT_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::openBlockScope()
{
	m_scopeFuncs.push_back({});

	// Create new block scope inside current function scope
	if (m_inFunctionDef)
	{
		yulAssert(
			!m_funcVars.empty(),
			"Proto fuzzer: Invalid data structure"
		);
		m_funcVars.back().push_back(vector<string>{});
		if (m_inForInitScope && m_forInitScopeExtEnabled)
		{
			yulAssert(
				!m_funcForLoopInitVars.empty(),
				"Proto fuzzer: Invalid data structure"
			);
			m_funcForLoopInitVars.back().push_back(vector<string>{});
		}
	}
	else
	{
		m_globalVars.push_back({});
		if (m_inForInitScope && m_forInitScopeExtEnabled)
			m_globalForLoopInitVars.push_back(vector<string>{});
	}
}

void ProtoConverter::openFunctionScope(vector<string> const& _funcParams)
{
	m_funcVars.push_back(vector<vector<string>>({_funcParams}));
	m_funcForLoopInitVars.push_back(vector<vector<string>>({}));
}

void ProtoConverter::updateFunctionMaps(string const& _var)
{
	unsigned erased = m_functionSigMap.erase(_var);

	for (auto const& i: m_functionDefMap)
		if (i.second == _var)
		{
			erased += m_functionDefMap.erase(i.first);
			break;
		}

	yulAssert(erased == 2, "Proto fuzzer: Function maps not updated");
}

void ProtoConverter::closeBlockScope()
{
	// Remove functions declared in the block that is going
	// out of scope from the global function map.
	for (auto const& f: m_scopeFuncs.back())
	{
		unsigned numFuncsRemoved = m_functions.size();
		m_functions.erase(remove(m_functions.begin(), m_functions.end(), f), m_functions.end());
		numFuncsRemoved -= m_functions.size();
		yulAssert(
			numFuncsRemoved == 1,
			"Proto fuzzer: Nothing or too much went out of scope"
		);
		updateFunctionMaps(f);
	}
	// Pop back the vector of scoped functions.
	if (!m_scopeFuncs.empty())
		m_scopeFuncs.pop_back();

	// If block belongs to function body, then remove
	// local variables in function body that are going out of scope.
	if (m_inFunctionDef)
	{
		yulAssert(!m_funcVars.empty(), "Proto fuzzer: Invalid data structure");
		if (!m_funcVars.back().empty())
			m_funcVars.back().pop_back();
	}
	// Remove variables declared in vanilla block from current
	// global scope.
	else
	{
		yulAssert(!m_globalVars.empty(), "Proto fuzzer: Invalid data structure");
		m_globalVars.pop_back();
	}
}

void ProtoConverter::closeFunctionScope()
{
	yulAssert(!m_funcVars.empty(), "Proto fuzzer: Invalid data structure");
	m_funcVars.pop_back();
	yulAssert(!m_funcForLoopInitVars.empty(), "Proto fuzzer: Invalid data structure");
	m_funcForLoopInitVars.pop_back();
}

void ProtoConverter::addVarsToScope(vector<string> const& _vars)
{
	// If we are in function definition, add the new vars to current function scope
	if (m_inFunctionDef)
	{
		// If we are directly in for-init block, add the newly created vars to the
		// stack of for-init variables.
		if (m_inForInitScope && m_forInitScopeExtEnabled)
		{
			yulAssert(
				!m_funcForLoopInitVars.empty() && !m_funcForLoopInitVars.back().empty(),
				"Proto fuzzer: Invalid data structure"
			);
			m_funcForLoopInitVars.back().back().insert(
				m_funcForLoopInitVars.back().back().end(),
				_vars.begin(),
				_vars.end()
			);
		}
		else
		{
			yulAssert(
				!m_funcVars.empty() && !m_funcVars.back().empty(),
				"Proto fuzzer: Invalid data structure"
			);
			m_funcVars.back().back().insert(
				m_funcVars.back().back().end(),
				_vars.begin(),
				_vars.end()
			);
		}
	}
	// If we are in a vanilla block, add the new vars to current global scope
	else
	{
		if (m_inForInitScope && m_forInitScopeExtEnabled)
		{
			yulAssert(
				!m_globalForLoopInitVars.empty(),
				"Proto fuzzer: Invalid data structure"
			);
			m_globalForLoopInitVars.back().insert(
				m_globalForLoopInitVars.back().end(),
				_vars.begin(),
				_vars.end()
			);
		}
		else
		{
			yulAssert(
				!m_globalVars.empty(),
				"Proto fuzzer: Invalid data structure"
			);
			m_globalVars.back().insert(
				m_globalVars.back().end(),
				_vars.begin(),
				_vars.end()
			);
		}
	}
}

void ProtoConverter::storeGlobals()
{
	if (m_globalVars.size() == 1)
	{
		unsigned i = 0;
		for (auto const& v: m_globalVars.back())
		{
			string loc = to_string(i * 32);
			m_output << Whiskers(R"(mstore(<loc>, <var>)<endl>)")
				("loc", loc)
				("var", v)
				("endl", "\n")
				.render();
			i++;
		}
	}
}

void ProtoConverter::visit(Block const& _x)
{
	openBlockScope();

	// Register function declarations in this scope unless this
	// scope belongs to for-init (in which function declarations
	// are forbidden).
	for (auto const& statement: _x.statements())
		if (statement.has_funcdef() && statement.funcdef().block().statements_size() > 0 && !m_inForInitScope)
			registerFunction(&statement.funcdef());

	if (_x.statements_size() > 0)
	{
		m_output << "{\n";
		bool wasForInitScopeExtEnabled = m_forInitScopeExtEnabled;
		for (auto const& st: _x.statements())
		{
			// If statement is block or introduces one and we are in for-init block
			// then temporarily disable scope extension if it is not already disabled.
			if (
				(st.has_blockstmt() || st.has_switchstmt() || st.has_ifstmt()) &&
				m_inForInitScope &&
				m_forInitScopeExtEnabled
			)
				m_forInitScopeExtEnabled = false;
			visit(st);
			m_forInitScopeExtEnabled = wasForInitScopeExtEnabled;
		}
		m_output << "}\n";
	}
	else
		m_output << "{}\n";
	closeBlockScope();
}

void ProtoConverter::registerFunction(FunctionDef const* _x)
{
	unsigned numInParams = _x->num_input_params() % s_modInputParams;
	unsigned numOutParams = _x->num_output_params() % s_modOutputParams;
	NumFunctionReturns numReturns;
	if (numOutParams == 0)
		numReturns = NumFunctionReturns::None;
	else if (numOutParams == 1)
		numReturns = NumFunctionReturns::Single;
	else
		numReturns = NumFunctionReturns::Multiple;

	// Generate function name
	string funcName = functionName(numReturns);

	// Register function
	auto ret = m_functionSigMap.emplace(make_pair(funcName, make_pair(numInParams, numOutParams)));
	yulAssert(ret.second, "Proto fuzzer: Function already exists.");
	m_functions.push_back(funcName);
	m_scopeFuncs.back().push_back(funcName);
	m_functionDefMap.emplace(make_pair(_x, funcName));
}

void ProtoConverter::fillFunctionCallInput(unsigned _numInParams)
{
	for (unsigned i = 0; i < _numInParams; i++)
	{
		// Throw a 4-sided dice to choose whether to populate function input
		// argument from a pseudo-randomly chosen slot in one of the following
		// locations: calldata, memory, storage, or Yul optimizer dictionary.
		unsigned diceValue = counter() % 4;
		// Pseudo-randomly choose one of the first ten 32-byte
		// aligned slots.
		string slot = to_string((counter() % 8) * 32);
		switch (diceValue)
		{
		case 0:
			m_output << "calldataload(" << slot << ")";
			break;
		case 1:
			m_output << "mload(" << slot << ")";
			break;
		case 2:
			m_output << "sload(" << slot << ")";
			break;
		case 3:
			// Call to dummyExpression() automatically picks a token
			// at a pseudo-random location.
			m_output << dummyExpression();
			break;
		}
		if (i < _numInParams - 1)
			m_output << ",";
	}
}

void ProtoConverter::saveFunctionCallOutput(vector<string> const& _varsVec)
{
	for (auto const& var: _varsVec)
	{
		// Flip a dice to choose whether to save output values
		// in storage or memory.
		bool coinFlip = counter() % 2 == 0;
		// Pseudo-randomly choose one of the first ten 32-byte
		// aligned slots.
		string slot = to_string((counter() % 10) * 32);
		if (coinFlip)
			m_output << "sstore(" << slot << ", " << var << ")\n";
		else
			m_output << "mstore(" << slot << ", " << var << ")\n";
	}
}

void ProtoConverter::createFunctionDefAndCall(
	FunctionDef const& _x,
	unsigned _numInParams,
	unsigned _numOutParams
)
{
	yulAssert(
		((_numInParams <= s_modInputParams - 1) && (_numOutParams <= s_modOutputParams - 1)),
		"Proto fuzzer: Too many function I/O parameters requested."
	);

	// Obtain function name
	yulAssert(m_functionDefMap.count(&_x), "Proto fuzzer: Unregistered function");
	string funcName = m_functionDefMap.at(&_x);

	vector<string> varsVec = {};
	m_output << "function " << funcName << "(";
	unsigned startIdx = counter();
	if (_numInParams > 0)
		varsVec = createVars(startIdx, startIdx + _numInParams);
	m_output << ")";

	vector<string> outVarsVec = {};
	// This creates -> x_n+1,...,x_r
	if (_numOutParams > 0)
	{
		m_output << " -> ";
		if (varsVec.empty())
		{
			yulAssert(_numInParams == 0, "Proto fuzzer: Input parameters not processed correctly");
			varsVec = createVars(startIdx, startIdx + _numOutParams);
		}
		else
		{
			outVarsVec = createVars(startIdx + _numInParams, startIdx + _numInParams + _numOutParams);
			varsVec.insert(varsVec.end(), outVarsVec.begin(), outVarsVec.end());
		}
	}
	yulAssert(varsVec.size() == _numInParams + _numOutParams, "Proto fuzzer: Function parameters not processed correctly");

	m_output << "\n";

	// If function definition is in for-loop body, update
	bool wasInForBody = m_inForBodyScope;
	m_inForBodyScope = false;

	bool wasInFunctionDef = m_inFunctionDef;
	m_inFunctionDef = true;

	// Create new function scope and add function input and return
	// parameters to it.
	openFunctionScope(varsVec);
	// Visit function body
	visit(_x.block());
	closeFunctionScope();

	m_inForBodyScope = wasInForBody;
	m_inFunctionDef = wasInFunctionDef;

	yulAssert(
		!m_inForInitScope,
		"Proto fuzzer: Trying to create function call inside a for-init block"
	);
}

void ProtoConverter::visit(FunctionDef const& _x)
{
	unsigned numInParams = _x.num_input_params() % s_modInputParams;
	unsigned numOutParams = _x.num_output_params() % s_modOutputParams;
	createFunctionDefAndCall(_x, numInParams, numOutParams);
}

void ProtoConverter::visit(PopStmt const& _x)
{
	m_output << "pop(";
	visit(_x.expr());
	m_output << ")\n";
}

void ProtoConverter::visit(LeaveStmt const&)
{
	m_output << "leave\n";
}

void ProtoConverter::visit(Program const& _x)
{
	// Initialize input size
	m_inputSize = _x.ByteSizeLong();
	m_output << "{\n";
	visit(_x.block());
	yulAssert(m_globalVars.size() <= 1, "Yul proto converter: Invalid global scope size");
	storeGlobals();
	m_output << "}\n";
}

string ProtoConverter::programToString(Program const& _input)
{
	visit(_input);
	return m_output.str();
}

std::string ProtoConverter::functionTypeToString(NumFunctionReturns _type)
{
	switch (_type)
	{
	case NumFunctionReturns::None:
		return "n";
	case NumFunctionReturns::Single:
		return "s";
	case NumFunctionReturns::Multiple:
		return "m";
	}
}
