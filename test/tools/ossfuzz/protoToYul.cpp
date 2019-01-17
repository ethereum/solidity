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

#include <ostream>
#include <sstream>

using namespace std;
using namespace yul::test::yul_fuzzer;

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, Literal const& _x)
{
	return _os << "(" << _x.val() << ")";
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, VarRef const& _x)
{
	return _os  << "x_" << (static_cast<uint32_t>(_x.varnum()) % 10);
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, Expression const& _x)
{
	if (_x.has_varref())
		return _os  << _x.varref();
	else if (_x.has_cons())
		return _os  << _x.cons();
	else if (_x.has_binop())
		return _os  << _x.binop();
	else if (_x.has_unop())
		return _os  << _x.unop();
	return _os << "";
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, BinaryOp const& _x)
{
	switch (_x.op())
	{
		case BinaryOp::ADD:
			_os << "add(";
			break;
		case BinaryOp::SUB:
			_os << "sub(";
			break;
		case BinaryOp::MUL:
			_os << "mul(";
			break;
		case BinaryOp::DIV:
			_os << "div(";
			break;
		case BinaryOp::MOD:
			_os << "mod(";
			break;
		case BinaryOp::XOR:
			_os << "xor(";
			break;
		case BinaryOp::AND:
			_os << "and(";
			break;
		case BinaryOp::OR:
			_os << "or(";
			break;
		case BinaryOp::EQ:
			_os << "eq(";
			break;
		case BinaryOp::LT:
			_os << "lt(";
			break;
		case BinaryOp::GT:
			_os << "gt(";
			break;
	}
	return _os << _x.left() << "," << _x.right() << ")";
}

// New var numbering starts from x_10 until x_16
ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, VarDecl const& _x)
{
	_os << "let x_" << ((_x.id() % 7) + 10) << " := " << _x.expr() << "\n";
	return _os;
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, TypedVarDecl const& _x)
{
	_os << "let x_" << ((_x.id() % 7) + 10);
	switch (_x.type())
	{
		case TypedVarDecl::BOOL:
			_os << ": bool := " << _x.expr() << " : bool\n";
			break;
		case TypedVarDecl::S8:
			_os << ": s8 := " << _x.expr() << " : s8\n";
			break;
		case TypedVarDecl::S32:
			_os << ": s32 := " << _x.expr() << " : s32\n";
			break;
		case TypedVarDecl::S64:
			_os << ": s64 := " << _x.expr() << " : s64\n";
			break;
		case TypedVarDecl::S128:
			_os << ": s128 := " << _x.expr() << " : s128\n";
			break;
		case TypedVarDecl::S256:
			_os << ": s256 := " << _x.expr() << " : s256\n";
			break;
		case TypedVarDecl::U8:
			_os << ": u8 := " << _x.expr() << " : u8\n";
			break;
		case TypedVarDecl::U32:
			_os << ": u32 := " << _x.expr() << " : u32\n";
			break;
		case TypedVarDecl::U64:
			_os << ": u64 := " << _x.expr() << " : u64\n";
			break;
		case TypedVarDecl::U128:
			_os << ": u128 := " << _x.expr() << " : u128\n";
			break;
		case TypedVarDecl::U256:
			_os << ": u256 := " << _x.expr() << " : u256\n";
			break;
	}
	return _os;
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, UnaryOp const& _x)
{
	switch (_x.op())
	{
		case UnaryOp::NOT:
			_os << "not(";
			break;
		case UnaryOp::MLOAD:
			_os << "mload(";
			break;
		case UnaryOp::SLOAD:
			_os << "sload(";
			break;
		case UnaryOp::ISZERO:
			_os << "iszero(";
			break;
	}
	_os  << _x.operand() << ")";
	return _os;
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, AssignmentStatement const& _x)
{
	return _os  << _x.ref_id() << " := " << _x.expr() << "\n";
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, IfStmt const& _x)
{
	return _os <<
	           "if " <<
	           _x.cond() <<
	           " " <<
	           _x.if_body();
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, StoreFunc const& _x)
{
	switch (_x.st())
	{
		case StoreFunc::MSTORE:
			_os << "mstore(" << _x.loc() << ", " << _x.val() << ")\n";
			break;
		case StoreFunc::SSTORE:
			_os << "sstore(" << _x.loc() << ", " << _x.val() << ")\n";
			break;
	}
	return _os;
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, Statement const& _x)
{
	if (_x.has_decl())
		return _os  << _x.decl();
	else if (_x.has_assignment())
		return _os  << _x.assignment();
	else if (_x.has_ifstmt())
		return _os  << _x.ifstmt();
	else if (_x.has_storage_func())
		return _os  << _x.storage_func();
	else if (_x.has_blockstmt())
		return _os << _x.blockstmt();
	return _os << "";
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, Block const& _x)
{
	if (_x.statements_size() > 0)
	{
		_os << "{\n";
		for (auto const& st: _x.statements())
			_os  << st;
		_os << "}\n";

	}
	return _os;
}

ostream& yul::test::yul_fuzzer::operator<<(ostream& _os, Function const& _x)
{
	_os << "{\n"
		<< "let a,b := foo(calldataload(0),calldataload(32),calldataload(64),calldataload(96),calldataload(128),"
		<< "calldataload(160),calldataload(192),calldataload(224))\n"
		<< "sstore(0, a)\n"
		<< "sstore(32, b)\n"
		<< "function foo(x_0, x_1, x_2, x_3, x_4, x_5, x_6, x_7) -> x_8, x_9\n"
		<< _x.statements()
		<< "}\n";
	return _os;
}

string yul::test::yul_fuzzer::functionToString(Function const& _input)
{
	ostringstream os;
	os << _input;
	return os.str();
}

string yul::test::yul_fuzzer::protoToYul(const uint8_t* _data, size_t _size)
{
	Function message;
	if (!message.ParsePartialFromArray(_data, _size))
		return "#error invalid proto\n";
	return functionToString(message);
}