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

EVMVersion ProtoConverter::evmVersionMapping(Program_Version const& _ver)
{
	switch (_ver)
	{
	case Program::HOMESTEAD:
		return EVMVersion::homestead();
	case Program::TANGERINE:
		return EVMVersion::tangerineWhistle();
	case Program::SPURIOUS:
		return EVMVersion::spuriousDragon();
	case Program::BYZANTIUM:
		return EVMVersion::byzantium();
	case Program::CONSTANTINOPLE:
		return EVMVersion::constantinople();
	case Program::PETERSBURG:
		return EVMVersion::petersburg();
	case Program::ISTANBUL:
		return EVMVersion::istanbul();
	case Program::BERLIN:
		return EVMVersion::berlin();
	}
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

bool ProtoConverter::functionCallNotPossible(FunctionCall_Returns _type)
{
	return _type == FunctionCall::SINGLE ||
		(_type == FunctionCall::MULTIASSIGN && !varDeclAvailable());
}

void ProtoConverter::visit(VarRef const& _x)
{
	if (m_inFunctionDef)
	{
		// Ensure that there is at least one variable declaration to reference in function scope.
		yulAssert(m_currentFuncVars.size() > 0, "Proto fuzzer: No variables to reference.");
		m_output << *m_currentFuncVars[static_cast<size_t>(_x.varnum()) % m_currentFuncVars.size()];
	}
	else
	{
		// Ensure that there is at least one variable declaration to reference in nested scopes.
		yulAssert(m_currentGlobalVars.size() > 0, "Proto fuzzer: No global variables to reference.");
		m_output << *m_currentGlobalVars[static_cast<size_t>(_x.varnum()) % m_currentGlobalVars.size()];
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
			m_output << dictionaryToken();
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
	case Expression::kFuncExpr:
		// FunctionCall must return a single value, otherwise
		// we output a trivial expression "1".
		if (_x.func_expr().ret() == FunctionCall::SINGLE)
			visit(_x.func_expr());
		else
			m_output << dictionaryToken();
		break;
	case Expression::kLowcall:
		visit(_x.lowcall());
		break;
	case Expression::kCreate:
		visit(_x.create());
		break;
	case Expression::kUnopdata:
		if (m_isObject)
			visit(_x.unopdata());
		else
			m_output << dictionaryToken();
		break;
	case Expression::EXPR_ONEOF_NOT_SET:
		m_output << dictionaryToken();
		break;
	}
}

void ProtoConverter::visit(BinaryOp const& _x)
{
	BinaryOp_BOp op = _x.op();

	if ((op == BinaryOp::SHL || op == BinaryOp::SHR || op == BinaryOp::SAR) &&
		!m_evmVersion.hasBitwiseShifting())
	{
		m_output << dictionaryToken();
		return;
	}

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
		yulAssert(m_evmVersion.hasBitwiseShifting(), "Proto fuzzer: Invalid evm version");
		m_output << "shr";
		break;
	case BinaryOp::SHL:
		yulAssert(m_evmVersion.hasBitwiseShifting(), "Proto fuzzer: Invalid evm version");
		m_output << "shl";
		break;
	case BinaryOp::SAR:
		yulAssert(m_evmVersion.hasBitwiseShifting(), "Proto fuzzer: Invalid evm version");
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

void ProtoConverter::visit(VarDecl const& _x)
{
	string varName = newVarName();
	m_output << "let " << varName << " := ";
	visit(_x.expr());
	m_output << "\n";
	scopeVariables({varName});
}

void ProtoConverter::visit(MultiVarDecl const& _x)
{
	m_output << "let ";
	vector<string> varNames;
	// We support up to 4 variables in a single
	// declaration statement.
	unsigned numVars = _x.num_vars() % 3 + 2;
	string delimiter = "";
	for (unsigned i = 0; i < numVars; i++)
	{
		string varName = newVarName();
		varNames.push_back(varName);
		m_output << delimiter << varName;
		if (i == 0)
			delimiter = ", ";
	}
	m_output << "\n";
	scopeVariables(varNames);
}

void ProtoConverter::visit(TypedVarDecl const& _x)
{
	string varName = newVarName();
	m_output << "let " << varName;
	switch (_x.type())
	{
	case TypedVarDecl::BOOL:
		m_output << ": bool := ";
		visit(_x.expr());
		m_output << " : bool\n";
		break;
	case TypedVarDecl::S8:
		m_output << ": s8 := ";
		visit(_x.expr());
		m_output << " : s8\n";
		break;
	case TypedVarDecl::S32:
		m_output << ": s32 := ";
		visit(_x.expr());
		m_output << " : s32\n";
		break;
	case TypedVarDecl::S64:
		m_output << ": s64 := ";
		visit(_x.expr());
		m_output << " : s64\n";
		break;
	case TypedVarDecl::S128:
		m_output << ": s128 := ";
		visit(_x.expr());
		m_output << " : s128\n";
		break;
	case TypedVarDecl::S256:
		m_output << ": s256 := ";
		visit(_x.expr());
		m_output << " : s256\n";
		break;
	case TypedVarDecl::U8:
		m_output << ": u8 := ";
		visit(_x.expr());
		m_output << " : u8\n";
		break;
	case TypedVarDecl::U32:
		m_output << ": u32 := ";
		visit(_x.expr());
		m_output << " : u32\n";
		break;
	case TypedVarDecl::U64:
		m_output << ": u64 := ";
		visit(_x.expr());
		m_output << " : u64\n";
		break;
	case TypedVarDecl::U128:
		m_output << ": u128 := ";
		visit(_x.expr());
		m_output << " : u128\n";
		break;
	case TypedVarDecl::U256:
		m_output << ": u256 := ";
		visit(_x.expr());
		m_output << " : u256\n";
		break;
	}
	// If we are inside a for-init block, there are two places
	// where the visited vardecl may have been defined:
	// - directly inside the for-init block
	// - inside a block within the for-init block
	// In the latter case, we don't scope extend.
	if (m_inFunctionDef)
	{
		// Variables declared directly in for-init block
		// are tracked separately because their scope
		// extends beyond the block they are defined in
		// to the rest of the for-loop statement.
		if (m_inForInitScope && m_forInitScopeExtEnabled)
		{
			yulAssert(
				!m_funcForLoopInitVars.empty() && !m_funcForLoopInitVars.back().empty(),
				"Proto fuzzer: Invalid operation"
			);
			m_funcForLoopInitVars.back().back().push_back(varName);
		}
		else
		{
			yulAssert(
				!m_funcVars.empty() && !m_funcVars.back().empty(),
				"Proto fuzzer: Invalid operation"
			);
			m_funcVars.back().back().push_back(varName);
		}
	}
	else
	{
		if (m_inForInitScope && m_forInitScopeExtEnabled)
		{
			yulAssert(
				!m_globalForLoopInitVars.empty(),
				"Proto fuzzer: Invalid operation"
			);
			m_globalForLoopInitVars.back().push_back(varName);
		}
		else
		{
			yulAssert(
				!m_globalVars.empty(),
				"Proto fuzzer: Invalid operation"
			);
			m_globalVars.back().push_back(varName);
		}
	}
}

void ProtoConverter::visit(UnaryOp const& _x)
{
	UnaryOp_UOp op = _x.op();

	// Replace calls to extcodehash on unsupported EVMs with a dictionary
	// token.
	if (op == UnaryOp::EXTCODEHASH && !m_evmVersion.hasExtCodeHash())
	{
		m_output << dictionaryToken();
		return;
	}

	switch (op)
	{
	case UnaryOp::NOT:
		m_output << "not";
		break;
	case UnaryOp::MLOAD:
		m_output << "mload";
		break;
	case UnaryOp::SLOAD:
		m_output << "sload";
		break;
	case UnaryOp::ISZERO:
		m_output << "iszero";
		break;
	case UnaryOp::CALLDATALOAD:
		m_output << "calldataload";
		break;
	case UnaryOp::EXTCODESIZE:
		m_output << "extcodesize";
		break;
	case UnaryOp::EXTCODEHASH:
		m_output << "extcodehash";
		break;
	case UnaryOp::BALANCE:
		m_output << "balance";
		break;
	case UnaryOp::BLOCKHASH:
		m_output << "blockhash";
		break;
	}
	m_output << "(";
	visit(_x.operand());
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
	case NullaryOp::PC:
		m_output << "pc()";
		break;
	case NullaryOp::MSIZE:
		m_output << "msize()";
		break;
	case NullaryOp::GAS:
		m_output << "gas()";
		break;
	case NullaryOp::CALLDATASIZE:
		m_output << "calldatasize()";
		break;
	case NullaryOp::CODESIZE:
		m_output << "codesize()";
		break;
	case NullaryOp::RETURNDATASIZE:
		// If evm supports returndatasize, we generate it. Otherwise,
		// we output a dictionary token.
		if (m_evmVersion.supportsReturndata())
			m_output << "returndatasize()";
		else
			m_output << dictionaryToken();
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
	case NullaryOp::TIMESTAMP:
		m_output << "timestamp()";
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
		// Replace calls to selfbalance() on unsupported EVMs with a dictionary
		// token.
		if (m_evmVersion.hasSelfBalance())
			m_output << "selfbalance()";
		else
			m_output << dictionaryToken();
		break;
	case NullaryOp::CHAINID:
		// Replace calls to chainid() on unsupported EVMs with a dictionary
		// token.
		if (m_evmVersion.hasChainID())
			m_output << "chainid()";
		else
			m_output << dictionaryToken();
		break;
	}
}

void ProtoConverter::visit(CopyFunc const& _x)
{
	CopyFunc_CopyType type = _x.ct();

	// datacopy() is valid only if we are inside
	// a Yul object.
	if (type == CopyFunc::DATA && !m_isObject)
		return;

	// We don't generate code if the copy function is returndatacopy
	// and the underlying evm does not support it.
	if (type == CopyFunc::RETURNDATA && !m_evmVersion.supportsReturndata())
		return;

	switch (type)
	{
	case CopyFunc::CALLDATA:
		m_output << "calldatacopy";
		break;
	case CopyFunc::CODE:
		m_output << "codecopy";
		break;
	case CopyFunc::RETURNDATA:
		yulAssert(m_evmVersion.supportsReturndata(), "Proto fuzzer: Invalid evm version");
		m_output << "returndatacopy";
		break;
	case CopyFunc::DATA:
		m_output << "datacopy";
		break;
	}
	m_output << "(";
	visit(_x.target());
	m_output << ", ";
	visit(_x.source());
	m_output << ", ";
	visit(_x.size());
	m_output << ")\n";
}

void ProtoConverter::visit(ExtCodeCopy const& _x)
{
	m_output << "extcodecopy";
	m_output << "(";
	visit(_x.addr());
	m_output << ", ";
	visit(_x.target());
	m_output << ", ";
	visit(_x.source());
	m_output << ", ";
	visit(_x.size());
	m_output << ")\n";
}

void ProtoConverter::visit(LogFunc const& _x)
{
	switch (_x.num_topics())
	{
	case LogFunc::ZERO:
		m_output << "log0";
		m_output << "(";
		visit(_x.pos());
		m_output << ", ";
		visit(_x.size());
		m_output << ")\n";
		break;
	case LogFunc::ONE:
		m_output << "log1";
		m_output << "(";
		visit(_x.pos());
		m_output << ", ";
		visit(_x.size());
		m_output << ", ";
		visit(_x.t1());
		m_output << ")\n";
		break;
	case LogFunc::TWO:
		m_output << "log2";
		m_output << "(";
		visit(_x.pos());
		m_output << ", ";
		visit(_x.size());
		m_output << ", ";
		visit(_x.t1());
		m_output << ", ";
		visit(_x.t2());
		m_output << ")\n";
		break;
	case LogFunc::THREE:
		m_output << "log3";
		m_output << "(";
		visit(_x.pos());
		m_output << ", ";
		visit(_x.size());
		m_output << ", ";
		visit(_x.t1());
		m_output << ", ";
		visit(_x.t2());
		m_output << ", ";
		visit(_x.t3());
		m_output << ")\n";
		break;
	case LogFunc::FOUR:
		m_output << "log4";
		m_output << "(";
		visit(_x.pos());
		m_output << ", ";
		visit(_x.size());
		m_output << ", ";
		visit(_x.t1());
		m_output << ", ";
		visit(_x.t2());
		m_output << ", ";
		visit(_x.t3());
		m_output << ", ";
		visit(_x.t4());
		m_output << ")\n";
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

void ProtoConverter::visitFunctionInputParams(FunctionCall const& _x, unsigned _numInputParams)
{
	// We reverse the order of function input visits since it helps keep this switch case concise.
	switch (_numInputParams)
	{
	case 4:
		visit(_x.in_param4());
		m_output << ", ";
		BOOST_FALLTHROUGH;
	case 3:
		visit(_x.in_param3());
		m_output << ", ";
		BOOST_FALLTHROUGH;
	case 2:
		visit(_x.in_param2());
		m_output << ", ";
		BOOST_FALLTHROUGH;
	case 1:
		visit(_x.in_param1());
		BOOST_FALLTHROUGH;
	case 0:
		break;
	default:
		yulAssert(false, "Proto fuzzer: Function call with too many input parameters.");
		break;
	}
}

bool ProtoConverter::functionValid(FunctionCall_Returns _type, unsigned _numOutParams)
{
	switch (_type)
	{
	case FunctionCall::ZERO:
		return _numOutParams == 0;
	case FunctionCall::SINGLE:
		return _numOutParams == 1;
	case FunctionCall::MULTIDECL:
	case FunctionCall::MULTIASSIGN:
		return _numOutParams > 1;
	}
}

void ProtoConverter::convertFunctionCall(
	FunctionCall const& _x,
	std::string _name,
	unsigned _numInParams,
	bool _newLine
)
{
	m_output << _name << "(";
	visitFunctionInputParams(_x, _numInParams);
	m_output << ")";
	if (_newLine)
		m_output << "\n";
}

vector<string> ProtoConverter::createVarDecls(unsigned _start, unsigned _end, bool _isAssignment)
{
	m_output << "let ";
	vector<string> varsVec = createVars(_start, _end);
	if (_isAssignment)
		m_output << " := ";
	else
		m_output << "\n";
	return varsVec;
}

void ProtoConverter::visit(FunctionCall const& _x)
{
	bool functionAvailable = m_functionSigMap.size() > 0;
	unsigned numInParams, numOutParams;
	string funcName;
	FunctionCall_Returns funcType = _x.ret();
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
		// If there are no functions available, calls to functions that
		// return a single value may be replaced by a dictionary token.
		if (funcType == FunctionCall::SINGLE)
			m_output << dictionaryToken();
		return;
	}

	// If function selected for function call does not meet interface
	// requirements (num output values) for the function type
	// specified, then we return early unless it is a function call
	// that returns a single value (which may be replaced by a
	// dictionary token.
	if (!functionValid(funcType, numOutParams))
	{
		if (funcType == FunctionCall::SINGLE)
			m_output << dictionaryToken();
		return;
	}

	// If we are here, it means that we have at least one valid
	// function for making the function call
	switch (funcType)
	{
	case FunctionCall::ZERO:
		convertFunctionCall(_x, funcName, numInParams);
		break;
	case FunctionCall::SINGLE:
		// Since functions that return a single value are used as expressions
		// we do not print a newline because it is done by the expression
		// visitor.
		convertFunctionCall(_x, funcName, numInParams, /*newLine=*/false);
		break;
	case FunctionCall::MULTIDECL:
	{
		// Ensure that the chosen function returns at most 4 values
		yulAssert(
			numOutParams <= 4,
			"Proto fuzzer: Function call with too many output params encountered."
		);

		// Obtain variable name suffix
		unsigned startIdx = counter();
		vector<string> varsVec = createVarDecls(
			startIdx,
			startIdx + numOutParams,
			/*isAssignment=*/true
		);

		// Create RHS of multi var decl
		convertFunctionCall(_x, funcName, numInParams);
		// Add newly minted vars in the multidecl statement to current scope
		addVarsToScope(varsVec);
		break;
	}
	case FunctionCall::MULTIASSIGN:
		// Ensure that the chosen function returns at most 4 values
		yulAssert(
			numOutParams <= 4,
			"Proto fuzzer: Function call with too many output params encountered."
		);

		// Convert LHS of multi assignment
		// We reverse the order of out param visits since the order does not matter.
		// This helps reduce the size of this switch statement.
		switch (numOutParams)
		{
		case 4:
			visit(_x.out_param4());
			m_output << ", ";
			BOOST_FALLTHROUGH;
		case 3:
			visit(_x.out_param3());
			m_output << ", ";
			BOOST_FALLTHROUGH;
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
		convertFunctionCall(_x, funcName, numInParams);
		break;
	}
}

void ProtoConverter::visit(LowLevelCall const& _x)
{
	LowLevelCall_Type type = _x.callty();

	// Generate staticcall if it is supported by the underlying evm
	if (type == LowLevelCall::STATICCALL && !m_evmVersion.hasStaticCall())
	{
		// Since staticcall is supposed to return 0 on success and 1 on
		// failure, we can use counter value to emulate it
		m_output << ((counter() % 2) ? "0" : "1");
		return;
	}

	switch (type)
	{
	case LowLevelCall::CALL:
		m_output << "call(";
		break;
	case LowLevelCall::CALLCODE:
		m_output << "callcode(";
		break;
	case LowLevelCall::DELEGATECALL:
		m_output << "delegatecall(";
		break;
	case LowLevelCall::STATICCALL:
		yulAssert(m_evmVersion.hasStaticCall(), "Proto fuzzer: Invalid evm version");
		m_output << "staticcall(";
		break;
	}
	visit(_x.gas());
	m_output << ", ";
	visit(_x.addr());
	m_output << ", ";
	if (type == LowLevelCall::CALL || type == LowLevelCall::CALLCODE)
	{
		visit(_x.wei());
		m_output << ", ";
	}
	visit(_x.in());
	m_output << ", ";
	visit(_x.insize());
	m_output << ", ";
	visit(_x.out());
	m_output << ", ";
	visit(_x.outsize());
	m_output << ")";
}

void ProtoConverter::visit(Create const& _x)
{
	Create_Type type = _x.createty();

	// Replace a call to create2 on unsupported EVMs with a dictionary
	// token.
	if (type == Create::CREATE2 && !m_evmVersion.hasCreate2())
	{
		m_output << dictionaryToken();
		return;
	}

	switch (type)
	{
	case Create::CREATE:
		m_output << "create(";
		break;
	case Create::CREATE2:
		m_output << "create2(";
		break;
	}
	visit(_x.wei());
	m_output << ", ";
	visit(_x.position());
	m_output << ", ";
	visit(_x.size());
	if (type == Create::CREATE2)
	{
		m_output << ", ";
		visit(_x.value());
	}
	m_output << ")";
}

void ProtoConverter::visit(IfStmt const& _x)
{
	m_output << "if ";
	visit(_x.cond());
	m_output << " ";
	visit(_x.if_body());
}

void ProtoConverter::visit(StoreFunc const& _x)
{
	switch (_x.st())
	{
	case StoreFunc::MSTORE:
		m_output << "mstore(";
		break;
	case StoreFunc::SSTORE:
		m_output << "sstore(";
		break;
	case StoreFunc::MSTORE8:
		m_output << "mstore8(";
		break;
	}
	visit(_x.loc());
	m_output << ", ";
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
	visit(_x.for_body());
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
	visit(_x.for_body());
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
		visit(_x.case_block());
	}
}

void ProtoConverter::visit(SwitchStmt const& _x)
{
	if (_x.case_stmt_size() > 0 || _x.has_default_block())
	{
		std::set<u256> s;
		m_switchLiteralSetPerScope.push(s);
		m_output << "switch ";
		visit(_x.switch_expr());
		m_output << "\n";

		for (auto const& caseStmt: _x.case_stmt())
			visit(caseStmt);

		m_switchLiteralSetPerScope.pop();

		if (_x.has_default_block())
		{
			m_output << "default ";
			visit(_x.default_block());
		}
	}
}

void ProtoConverter::visit(StopInvalidStmt const& _x)
{
	switch (_x.stmt())
	{
	case StopInvalidStmt::STOP:
		m_output << "stop()\n";
		break;
	case StopInvalidStmt::INVALID:
		m_output << "invalid()\n";
		break;
	}
}

void ProtoConverter::visit(RetRevStmt const& _x)
{
	switch (_x.stmt())
	{
	case RetRevStmt::RETURN:
		m_output << "return";
		break;
	case RetRevStmt::REVERT:
		m_output << "revert";
		break;
	}
	m_output << "(";
	visit(_x.pos());
	m_output << ", ";
	visit(_x.size());
	m_output << ")\n";
}

void ProtoConverter::visit(SelfDestructStmt const& _x)
{
	m_output << "selfdestruct";
	m_output << "(";
	visit(_x.addr());
	m_output << ")\n";
}

void ProtoConverter::visit(TerminatingStmt const& _x)
{
	switch (_x.term_oneof_case())
	{
	case TerminatingStmt::kStopInvalid:
		visit(_x.stop_invalid());
		break;
	case TerminatingStmt::kRetRev:
		visit(_x.ret_rev());
		break;
	case TerminatingStmt::kSelfDes:
		visit(_x.self_des());
		break;
	case TerminatingStmt::TERM_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(UnaryOpData const& _x)
{
	switch (_x.op())
	{
	case UnaryOpData::SIZE:
		m_output << Whiskers(R"(datasize("<id>"))")
			("id", getObjectIdentifier(_x.identifier()))
			.render();
		break;
	case UnaryOpData::OFFSET:
		m_output << Whiskers(R"(dataoffset("<id>"))")
			("id", getObjectIdentifier(_x.identifier()))
			.render();
		break;
	}
}

void ProtoConverter::visit(Statement const& _x)
{
	switch (_x.stmt_oneof_case())
	{
	case Statement::kDecl:
		visit(_x.decl());
		break;
	case Statement::kAssignment:
		// Create an assignment statement only if there is at least one variable
		// declaration that is in scope.
		if (varDeclAvailable())
			visit(_x.assignment());
		break;
	case Statement::kIfstmt:
		if (_x.ifstmt().if_body().statements_size() > 0)
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
		if (_x.forstmt().for_body().statements_size() > 0)
			visit(_x.forstmt());
		break;
	case Statement::kBoundedforstmt:
		if (_x.boundedforstmt().for_body().statements_size() > 0)
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
	case Statement::kLogFunc:
		visit(_x.log_func());
		break;
	case Statement::kCopyFunc:
		visit(_x.copy_func());
		break;
	case Statement::kExtcodeCopy:
		visit(_x.extcode_copy());
		break;
	case Statement::kTerminatestmt:
		visit(_x.terminatestmt());
		break;
	case Statement::kFunctioncall:
		// Return early if a function call cannot be created
		if (functionCallNotPossible(_x.functioncall().ret()))
			return;
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
	case Statement::kMultidecl:
		visit(_x.multidecl());
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
		string slot = to_string((counter() % 10) * 32);
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
			// Call to dictionaryToken() automatically picks a token
			// at a pseudo-random location.
			m_output << dictionaryToken();
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

void ProtoConverter::createFunctionCall(
	string _funcName,
	unsigned _numInParams,
	unsigned _numOutParams
)
{
	vector<string> varsVec{};
	if (_numOutParams > 0)
	{
		unsigned startIdx = counter();
		// Prints the following to output stream "let x_i,...,x_n := "
		varsVec = createVarDecls(
			startIdx,
			startIdx + _numOutParams,
			/*isAssignment=*/true
		);
	}

	// Call the function with the correct number of input parameters
	m_output << _funcName << "(";
	if (_numInParams > 0)
		fillFunctionCallInput(_numInParams);
	m_output << ")\n";

	if (!varsVec.empty())
	{
		// Save values returned by function so that they are reflected
		// in the interpreter trace.
		saveFunctionCallOutput(varsVec);
		// Add newly minted vars to current scope
		addVarsToScope(varsVec);
	}
	else
		yulAssert(_numOutParams == 0, "Proto fuzzer: Function return value not saved");
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
	if (_x.force_call())
		createFunctionCall(funcName, _numInParams, _numOutParams);
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

string ProtoConverter::getObjectIdentifier(unsigned _x)
{
	unsigned currentId = currentObjectId();
	yulAssert(m_objectScopeTree.size() > currentId, "Proto fuzzer: Error referencing object");
	std::vector<std::string> objectIdsInScope = m_objectScopeTree[currentId];
	return objectIdsInScope[_x % objectIdsInScope.size()];
}

void ProtoConverter::visit(Code const& _x)
{
	m_output << "code {\n";
	visit(_x.block());
	m_output << "}\n";
}

void ProtoConverter::visit(Data const& _x)
{
	// TODO: Generate random data block identifier
	m_output << "data \"" << s_dataIdentifier << "\" hex\"" << createHex(_x.hex()) << "\"\n";
}

void ProtoConverter::visit(Object const& _x)
{
	// object "object<n>" {
	// ...
	// }
	m_output << "object " << newObjectId() << " {\n";
	visit(_x.code());
	if (_x.has_data())
		visit(_x.data());
	if (_x.has_sub_obj())
		visit(_x.sub_obj());
	m_output << "}\n";
}

void ProtoConverter::buildObjectScopeTree(Object const& _x)
{
	// Identifies object being visited
	string objectId = newObjectId(false);
	vector<string> node{objectId};
	if (_x.has_data())
		node.push_back(s_dataIdentifier);
	if (_x.has_sub_obj())
	{
		// Identifies sub object whose numeric suffix is
		// m_objectId
		string subObjectId = "object" + to_string(m_objectId);
		node.push_back(subObjectId);
		// TODO: Add sub-object to object's ancestors once
		// nested access is implemented.
		m_objectScopeTree.push_back(node);
		buildObjectScopeTree(_x.sub_obj());
	}
	else
		m_objectScopeTree.push_back(node);
}

void ProtoConverter::visit(Program const& _x)
{
	// Initialize input size
	m_inputSize = _x.ByteSizeLong();

	// Record EVM Version
	m_evmVersion = evmVersionMapping(_x.ver());

	// Program is either a Yul object or a block of
	// statements.
	switch (_x.program_oneof_case())
	{
	case Program::kBlock:
		m_output << "{\n";
		visit(_x.block());
		m_output << "}\n";
		break;
	case Program::kObj:
		m_isObject = true;
		buildObjectScopeTree(_x.obj());
		// Reset object id counter
		m_objectId = 0;
		visit(_x.obj());
		break;
	case Program::PROGRAM_ONEOF_NOT_SET:
		// {} is a trivial Yul program
		m_output << "{}";
		break;
	}
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
