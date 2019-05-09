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
#include <libsolidity/codegen/YulUtilFunctions.h>
#include <boost/range/algorithm_ext/erase.hpp>
#include <libyul/Exceptions.h>

using namespace std;
using namespace yul::test::yul_fuzzer;
using namespace dev::solidity;

string ProtoConverter::createHex(string const& _hexBytes) const
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
	if (tmp.empty())
		tmp = "1";
	return tmp;
}

string ProtoConverter::createAlphaNum(string const& _strBytes) const
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

bool ProtoConverter::isCaseLiteralUnique(Literal const& _x)
{
	std::string tmp;
	bool isUnique = false;
	bool isEmptyString = false;
	switch (_x.literal_oneof_case())
	{
	case Literal::kIntval:
		tmp = std::to_string(_x.intval());
		break;
	case Literal::kHexval:
		tmp = "0x" + createHex(_x.hexval());
		break;
	case Literal::kStrval:
		tmp = createAlphaNum(_x.strval());
		if (tmp.empty())
		{
			isEmptyString = true;
			tmp = std::to_string(0);
		}
		else
			tmp = "\"" + tmp + "\"";
		break;
	case Literal::LITERAL_ONEOF_NOT_SET:
		tmp = std::to_string(1);
		break;
	}
	if (!_x.has_strval() || isEmptyString)
		isUnique = m_switchLiteralSetPerScope.top().insert(dev::u256(tmp)).second;
	else
		isUnique = m_switchLiteralSetPerScope.top().insert(
				dev::u256(dev::h256(tmp, dev::h256::FromBinary, dev::h256::AlignLeft))).second;
	return isUnique;
}

void ProtoConverter::visit(Literal const& _x)
{
	switch (_x.literal_oneof_case())
	{
	case Literal::kIntval:
		m_output << _x.intval();
		break;
	case Literal::kHexval:
		m_output << "0x" << createHex(_x.hexval());
		break;
	case Literal::kStrval:
		m_output << "\"" << createAlphaNum(_x.strval()) << "\"";
		break;
	case Literal::LITERAL_ONEOF_NOT_SET:
		m_output << "1";
		break;
	}
}

// Reference any index in [0, m_numLiveVars-1]
void ProtoConverter::visit(VarRef const& _x)
{
	yulAssert(m_numLiveVars > 0, "Proto fuzzer: No variables to reference.");
	m_output  << "x_" << (static_cast<uint32_t>(_x.varnum()) % m_numLiveVars);
}

void ProtoConverter::visit(Expression const& _x)
{
	switch (_x.expr_oneof_case())
	{
	case Expression::kVarref:
		visit(_x.varref());
		break;
	case Expression::kCons:
		visit(_x.cons());
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
		visit(_x.func_expr());
		break;
	case Expression::EXPR_ONEOF_NOT_SET:
		m_output << "1";
		break;
	}
}

void ProtoConverter::visit(BinaryOp const& _x)
{
	switch (_x.op())
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

void ProtoConverter::visit(VarDecl const& _x)
{
	m_output << "let x_" << m_numLiveVars << " := ";
	visit(_x.expr());
	m_numVarsPerScope.top()++;
	m_numLiveVars++;
	m_output << "\n";
}

void ProtoConverter::visit(EmptyVarDecl const&)
{
	m_output << "let x_" << m_numLiveVars++ << "\n";
	m_numVarsPerScope.top()++;
}

void ProtoConverter::visit(MultiVarDecl const& _x)
{
	size_t funcId = (static_cast<size_t>(_x.func_index()) % m_functionVecMultiReturnValue.size());

	int numInParams = m_functionVecMultiReturnValue.at(funcId).first;
	int numOutParams = m_functionVecMultiReturnValue.at(funcId).second;

	// Ensure that the chosen function returns at least 2 and at most 4 values
	yulAssert(
		((numOutParams >= 2) && (numOutParams <= 4)),
		"Proto fuzzer: Multi variable declaration calls a function with either too few or too many output params."
	);

	// We must start variable numbering past the number of live variables at this point in time.
	// This creates let x_p,..., x_k :=
	// (k-p)+1 = numOutParams
	m_output <<
		"let " <<
		YulUtilFunctions::suffixedVariableNameList("x_", m_numLiveVars, m_numLiveVars + numOutParams) <<
		" := ";

	// Create RHS of multi var decl
	m_output << "foo_" << functionTypeToString(NumFunctionReturns::Multiple) << "_" << funcId;
	m_output << "(";
	visitFunctionInputParams(_x, numInParams);
	m_output << ")\n";
	// Update live variables in scope and in total to account for the variables created by this
	// multi variable declaration.
	m_numVarsPerScope.top() += numOutParams;
	m_numLiveVars += numOutParams;
}

void ProtoConverter::visit(TypedVarDecl const& _x)
{
	m_output << "let x_" << m_numLiveVars;
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
	m_numVarsPerScope.top()++;
	m_numLiveVars++;
}

void ProtoConverter::visit(UnaryOp const& _x)
{
	switch (_x.op())
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
		m_output << "returndatasize()";
		break;
	}
}

void ProtoConverter::visit(CopyFunc const& _x)
{
	switch (_x.ct())
	{
	case CopyFunc::CALLDATA:
		m_output << "calldatacopy";
		break;
	case CopyFunc::CODE:
		m_output << "codecopy";
		break;
	case CopyFunc::RETURNDATA:
		m_output << "returndatacopy";
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

// Called at the time function call is being made
template <class T>
void ProtoConverter::visitFunctionInputParams(T const& _x, unsigned _numInputParams)
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

void ProtoConverter::visit(MultiAssignment const& _x)
{
	size_t funcId = (static_cast<size_t>(_x.func_index()) % m_functionVecMultiReturnValue.size());
	unsigned numInParams = m_functionVecMultiReturnValue.at(funcId).first;
	unsigned numOutParams = m_functionVecMultiReturnValue.at(funcId).second;
	yulAssert(
		((numOutParams >= 2) && (numOutParams <= 4)),
		"Proto fuzzer: Multi assignment calls a function that has either too many or too few output parameters."
	);

	// Convert LHS of multi assignment
	// We reverse the order of out param visits since the order does not matter. This helps reduce the size of this
	// switch statement.
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
		yulAssert(false, "Proto fuzzer: Function call with too many input parameters.");
		break;
	}
	m_output << " := ";

	// Convert RHS of multi assignment
	m_output << "foo_" << functionTypeToString(NumFunctionReturns::Multiple) << "_" << funcId;
	m_output << "(";
	visitFunctionInputParams(_x, numInParams);
	m_output << ")\n";
}

void ProtoConverter::visit(FunctionCallNoReturnVal const& _x)
{
	size_t funcId = (static_cast<size_t>(_x.func_index()) % m_functionVecNoReturnValue.size());
	unsigned numInParams = m_functionVecNoReturnValue.at(funcId);
	m_output << "foo_" << functionTypeToString(NumFunctionReturns::None) << "_" << funcId;
	m_output << "(";
	visitFunctionInputParams(_x, numInParams);
	m_output << ")\n";
}

void ProtoConverter::visit(FunctionCallSingleReturnVal const& _x)
{
	size_t funcId = (static_cast<size_t>(_x.func_index()) % m_functionVecSingleReturnValue.size());
	unsigned numInParams = m_functionVecSingleReturnValue.at(funcId);
	m_output << "foo_" << functionTypeToString(NumFunctionReturns::Single) << "_" << funcId;
	m_output << "(";
	visitFunctionInputParams(_x, numInParams);
	m_output << ")";
}

void ProtoConverter::visit(FunctionCall const& _x)
{
	switch (_x.functioncall_oneof_case())
	{
	case FunctionCall::kCallZero:
		visit(_x.call_zero());
		break;
	case FunctionCall::kCallMultidecl:
		visit(_x.call_multidecl());
		break;
	case FunctionCall::kCallMultiassign:
		visit(_x.call_multiassign());
		break;
	case FunctionCall::FUNCTIONCALL_ONEOF_NOT_SET:
		break;
	}
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
	// Boilerplate for loop that limits the number of iterations to a maximum of 4.
	// TODO: Generalize for loop init, condition, and post blocks.
	std::string loopVarName("i_" + std::to_string(m_numNestedForLoops++));
	m_output << "for { let " << loopVarName << " := 0 } "
		<< "lt(" << loopVarName << ", 0x60) "
		<< "{ " << loopVarName << " := add(" << loopVarName << ", 0x20) } ";
	m_inForScope.push(true);
	visit(_x.for_body());
	m_inForScope.pop();
	--m_numNestedForLoops;
}

void ProtoConverter::visit(CaseStmt const& _x)
{
	// Silently ignore duplicate case literals
	if (isCaseLiteralUnique(_x.case_lit()))
	{
		m_output << "case ";
		visit(_x.case_lit());
		m_output << " ";
		visit(_x.case_block());
	}
}

void ProtoConverter::visit(SwitchStmt const& _x)
{
	if (_x.case_stmt_size() > 0 || _x.has_default_block())
	{
		std::set<dev::u256> s;
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

void ProtoConverter::visit(Statement const& _x)
{
	switch (_x.stmt_oneof_case())
	{
	case Statement::kDecl:
		visit(_x.decl());
		break;
	case Statement::kAssignment:
		visit(_x.assignment());
		break;
	case Statement::kIfstmt:
		visit(_x.ifstmt());
		break;
	case Statement::kStorageFunc:
		visit(_x.storage_func());
		break;
	case Statement::kBlockstmt:
		visit(_x.blockstmt());
		break;
	case Statement::kForstmt:
		visit(_x.forstmt());
		break;
	case Statement::kSwitchstmt:
		visit(_x.switchstmt());
		break;
	case Statement::kBreakstmt:
		if (m_inForScope.top())
			m_output << "break\n";
		break;
	case Statement::kContstmt:
		if (m_inForScope.top())
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
		visit(_x.functioncall());
		break;
	case Statement::STMT_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(Block const& _x)
{
	if (_x.statements_size() > 0)
	{
		m_numVarsPerScope.push(0);
		m_output << "{\n";
		for (auto const& st: _x.statements())
			visit(st);
		m_output << "}\n";
		m_numLiveVars -= m_numVarsPerScope.top();
		m_numVarsPerScope.pop();
	}
	else
		m_output << "{}\n";
}

void ProtoConverter::visit(SpecialBlock const& _x)
{
	m_numVarsPerScope.push(0);
	m_output << "{\n";
	visit(_x.var());
	if (_x.statements_size() > 0)
		for (auto const& st: _x.statements())
			visit(st);
	m_numLiveVars -= m_numVarsPerScope.top();
	m_numVarsPerScope.pop();
	m_output << "}\n";
}

template <class T>
void ProtoConverter::createFunctionDefAndCall(T const& _x, unsigned _numInParams, unsigned _numOutParams, NumFunctionReturns _type)
{
	yulAssert(
		((_numInParams <= modInputParams - 1) && (_numOutParams <= modOutputParams - 1)),
		"Proto fuzzer: Too many function I/O parameters requested."
	);

	// At the time of function definition creation, the number of live variables must be 0.
	// This is because we always create only as many variables as we need within function scope.
	yulAssert(m_numLiveVars == 0, "Proto fuzzer: Unused live variable found.");

	// Signature
	// This creates function foo_<noreturn|singlereturn|multireturn>_<m_numFunctionSets>(x_0,...,x_n)
	m_output << "function foo_" << functionTypeToString(_type) << "_" << m_numFunctionSets;
	m_output << "(";
	if (_numInParams > 0)
		m_output << YulUtilFunctions::suffixedVariableNameList("x_", 0, _numInParams);
	m_output << ")";

	// Book keeping for variables in function scope and in nested scopes
	m_numVarsPerScope.push(_numInParams);
	m_numLiveVars += _numInParams;

	// This creates -> x_n+1,...,x_r
	if (_numOutParams > 0)
	{
		m_output << " -> " << YulUtilFunctions::suffixedVariableNameList("x_", _numInParams, _numInParams + _numOutParams);
		// More bookkeeping
		m_numVarsPerScope.top() += _numOutParams;
		m_numLiveVars += _numOutParams;
	}
	m_output << "\n";

	// Body
	visit(_x.statements());

	// Ensure that variable stack is balanced
	m_numLiveVars -= m_numVarsPerScope.top();
	m_numVarsPerScope.pop();
	yulAssert(m_numLiveVars == 0, "Proto fuzzer: Variable stack after function definition is unbalanced.");

	// Manually create a multi assignment using global variables
	// This prints a_0, ..., a_k-1 for this function that returns "k" values
	if (_numOutParams > 0)
		m_output << YulUtilFunctions::suffixedVariableNameList("a_", 0, _numOutParams) << " := ";

	// Call the function with the correct number of input parameters via calls to calldataload with
	// incremental addresses.
	m_output << "foo_" << functionTypeToString(_type) << "_" << std::to_string(m_numFunctionSets);
	m_output << "(";
	for (unsigned i = 0; i < _numInParams; i++)
	{
		m_output << "calldataload(" << std::to_string(i*32) << ")";
		if (i < _numInParams - 1)
			m_output << ",";
	}
	m_output << ")\n";

	for (unsigned i = 0; i < _numOutParams; i++)
		m_output << "sstore(" << std::to_string(i*32) << ", a_" << std::to_string(i) << ")\n";
}

void ProtoConverter::visit(FunctionDefinitionNoReturnVal const& _x)
{
	unsigned numInParams = _x.num_input_params() % modInputParams;
	unsigned numOutParams = 0;
	createFunctionDefAndCall(_x, numInParams, numOutParams, NumFunctionReturns::None);
}

void ProtoConverter::visit(FunctionDefinitionSingleReturnVal const& _x)
{
	unsigned numInParams = _x.num_input_params() % modInputParams;
	unsigned numOutParams = 1;
	createFunctionDefAndCall(_x, numInParams, numOutParams, NumFunctionReturns::Single);
}

void ProtoConverter::visit(FunctionDefinitionMultiReturnVal const& _x)
{
	unsigned numInParams = _x.num_input_params() % modInputParams;
	// Synthesize at least 2 return parameters and at most (modOutputParams - 1)
	unsigned numOutParams = std::max<unsigned>(2, _x.num_output_params() % modOutputParams);
	createFunctionDefAndCall(_x, numInParams, numOutParams, NumFunctionReturns::Multiple);
}

void ProtoConverter::visit(FunctionDefinition const& _x)
{
	visit(_x.fd_zero());
	visit(_x.fd_one());
	visit(_x.fd_multi());
	m_numFunctionSets++;
}

void ProtoConverter::visit(Program const& _x)
{
	/* Program template is as follows
	 *      Four Globals a_0, a_1, a_2, and a_3 to hold up to four function return values
	 *
	 *      Repeated function definitions followed by function calls of the respective function
	 *          Example: function foo(x_0) -> x_1 {}
	 *                   a_0 := foo(calldataload(0))
	 *                   sstore(0, a_0)
	 */
	m_output << "{\n";
	// Create globals at the beginning
	// This creates let a_0, a_1, a_2, a_3 (followed by a new line)
	m_output << "let " << YulUtilFunctions::suffixedVariableNameList("a_", 0, modOutputParams - 1) << "\n";
	// Register function interface. Useful while visiting multi var decl/assignment statements.
	for (auto const& f: _x.funcs())
		registerFunction(f);

	for (auto const& f: _x.funcs())
		visit(f);

	yulAssert((unsigned)_x.funcs_size() == m_numFunctionSets, "Proto fuzzer: Functions not correctly registered.");
	m_output << "}\n";
}

string ProtoConverter::programToString(Program const& _input)
{
	visit(_input);
	return m_output.str();
}

void ProtoConverter::registerFunction(FunctionDefinition const& _x)
{
	// No return and single return functions explicitly state the number of values returned
	registerFunction(_x.fd_zero(), NumFunctionReturns::None);
	registerFunction(_x.fd_one(), NumFunctionReturns::Single);
	// A multi return function can have between two and (modOutputParams - 1) parameters
	unsigned numOutParams = std::max<unsigned>(2, _x.fd_multi().num_output_params() % modOutputParams);
	registerFunction(_x.fd_multi(), NumFunctionReturns::Multiple, numOutParams);
}

std::string ProtoConverter::functionTypeToString(NumFunctionReturns _type)
{
	switch (_type)
	{
	case NumFunctionReturns::None:
		return "noreturn";
	case NumFunctionReturns::Single:
		return "singlereturn";
	case NumFunctionReturns::Multiple:
		return "multireturn";
	}
}