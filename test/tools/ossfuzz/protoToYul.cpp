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
#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace yul::test::yul_fuzzer;

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

// Reference any index in [0, m_numLiveVars-1] or [0, m_numLiveVars)
void ProtoConverter::visit(VarRef const& _x)
{
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

// New var numbering starts from x_10
void ProtoConverter::visit(VarDecl const& _x)
{
	m_output << "let x_" << m_numLiveVars << " := ";
	visit(_x.expr());
	m_numVarsPerScope.top()++;
	m_numLiveVars++;
	m_output << "\n";
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
	}
	m_output << "(";
	visit(_x.operand());
	m_output << ")";
}

void ProtoConverter::visit(AssignmentStatement const& _x)
{
	visit(_x.ref_id());
	m_output << " := ";
	visit(_x.expr());
	m_output << "\n";
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
	}
	visit(_x.loc());
	m_output << ", ";
	visit(_x.val());
	m_output << ")\n";
}

void ProtoConverter::visit(ForStmt const& _x)
{
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

void ProtoConverter::visit(Function const& _x)
{
	m_output << "{\n"
	    << "let a,b := foo(calldataload(0),calldataload(32),calldataload(64),calldataload(96),calldataload(128),"
	    << "calldataload(160),calldataload(192),calldataload(224))\n"
	    << "sstore(0, a)\n"
	    << "sstore(32, b)\n"
	    << "function foo(x_0, x_1, x_2, x_3, x_4, x_5, x_6, x_7) -> x_8, x_9\n";
	visit(_x.statements());
	m_output << "}\n";
}

string ProtoConverter::functionToString(Function const& _input)
{
	visit(_input);
	return m_output.str();
}

string ProtoConverter::protoToYul(const uint8_t* _data, size_t _size)
{
	Function message;
	if (!message.ParsePartialFromArray(_data, _size))
		return "#error invalid proto\n";
	return functionToString(message);
}
