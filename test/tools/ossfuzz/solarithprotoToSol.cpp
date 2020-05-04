#include <test/tools/ossfuzz/solarithprotoToSol.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/Whiskers.h>

using namespace solidity::test::solarithfuzzer;
using namespace solidity;
using namespace solidity::util;
using namespace std;

string ProtoConverter::programToString(Program const& _program)
{
	m_rand = make_unique<SolRandomNumGenerator>(
		SolRandomNumGenerator(_program.seed())
	);
	return visit(_program);
}

string ProtoConverter::visit(Program const& _program)
{
	Whiskers p(R"(pragma solidity >= 0.0.0;)");
	Whiskers c(R"(contract C {)");
	Whiskers t(R"(function test() public returns (<type>)<body>)");
	m_returnType = signString(_program.s());
	t("type", m_returnType);
	t("body", visit(_program.b()));
	return p.render()
		+ '\n'
		+ c.render()
		+ '\n'
		+ '\t'
		+ t.render()
		+ '\n'
		+ '}';
}

string ProtoConverter::visit(Return const& _ret)
{
	Whiskers r(R"(return <type>(<expr>);)");
	r("expr", visit(_ret.e()));
	r("type", m_returnType);
	return "\t\t" + r.render() + '\n';
}

string ProtoConverter::visit(Block const& _block)
{
	ostringstream blockStr;
	blockStr << '\n'
		<< '\t'
		<< '{'
		<< '\n';
	for (auto const& s: _block.s())
		blockStr << visit(s);
	blockStr << visit(_block.r());
	blockStr << '\n'
		<< '\t'
		<< '}';
	return blockStr.str();
}

string ProtoConverter::visit(Statement const& _stmt)
{
	switch (_stmt.stmt_oneof_case())
	{
	case Statement::kVd:
		return visit(_stmt.vd());
	case Statement::kA:
		if (varAvailable())
			return visit(_stmt.a());
		else
			return "";
	case Statement::kU:
		if (varAvailable())
			return visit(_stmt.u());
		else
			return "";
	case Statement::kB:
		if (varAvailable())
			return visit(_stmt.b());
		else
			return "";
	case Statement::STMT_ONEOF_NOT_SET:
		return "";
	}
}

string ProtoConverter::visit(VarDecl const& _vardecl)
{
	Whiskers v(R"(<type> <varName> = <type>(<value>);)");
	string type = visit(_vardecl.t());
	string varName = newVarName();
	m_varTypeMap.emplace(varName, pair(typeSign(_vardecl.t()), type));
	v("type", type);
	v("varName", varName);
	v("value", visit(_vardecl.value()));
	incrementVarCounter();
	return "\t\t" + v.render() + '\n';
}

string ProtoConverter::visit(Type const& _type)
{
	return signString(_type.s()) + widthString(_type.bytewidth());
}

string ProtoConverter::visit(UnaryOpStmt const& _uop)
{
	switch (_uop.op())
	{
	case UnaryOpStmt_Op_POSTINC:
		return "\t\t" + visit(_uop.v()) + "++;\n";
	case UnaryOpStmt_Op_POSTDEC:
		return "\t\t" + visit(_uop.v()) + "--;\n";
	case UnaryOpStmt_Op_PREINC:
		return "\t\t++" + visit(_uop.v()) + ";\n";
	case UnaryOpStmt_Op_PREDEC:
		return "\t\t--" + visit(_uop.v()) + ";\n";
	}
}

string ProtoConverter::visit(BinaryOpStmt const& _bopstmt)
{
	string op{};
	switch (_bopstmt.op())
	{
	case BinaryOpStmt::ADDSELF:
		op = " += ";
		break;
	case BinaryOpStmt::SUBSELF:
		op = " -= ";
		break;
	case BinaryOpStmt::MULSELF:
		op = " *= ";
		break;
	case BinaryOpStmt::DIVSELF:
		op = " /= ";
		break;
	case BinaryOpStmt::MODSELF:
		op = " %= ";
		break;
	case BinaryOpStmt::SHLSELF:
		op = " <<= ";
		break;
	case BinaryOpStmt::SHRSELF:
		op = " >>= ";
		break;
	}
	string left = visit(_bopstmt.left());
	string right = visit(_bopstmt.right());

	string leftSignString = m_varTypeMap[left].second;
	string rightSignString = m_exprSignMap[&_bopstmt.right()].second;
	if (leftSignString != rightSignString)
		right = leftSignString + '(' + right + ')';
	return "\t\t" + left + op + right + ";\n";
}

string ProtoConverter::visit(BinaryOp const& _bop)
{
	string op{};
	switch (_bop.op())
	{
	case BinaryOp_Op_ADD:
		op = " + ";
		break;
	case BinaryOp_Op_SUB:
		op = " - ";
		break;
	case BinaryOp_Op_MUL:
		op = " * ";
		break;
	case BinaryOp_Op_DIV:
		op = " / ";
		break;
	case BinaryOp_Op_MOD:
		op = " % ";
		break;
	case BinaryOp_Op_EXP:
		op = " ** ";
		break;
	case BinaryOp_Op_SHL:
		op = " << ";
		break;
	case BinaryOp_Op_SHR:
		op = " >> ";
		break;
	}
	auto left = visit(_bop.left());
	auto right = visit(_bop.right());

	Sign leftSign = m_exprSignMap[&_bop.left()].first;
	string leftSignString = m_exprSignMap[&_bop.left()].second;
	Sign rightSign = m_exprSignMap[&_bop.right()].first;
	string rightSignString = m_exprSignMap[&_bop.right()].second;

	bool expOrMod = binaryOperandExp(_bop.op());
	bool expSignChange = expOrMod && rightSign == Sign::Signed;
	if (expSignChange)
		right = Whiskers(R"(uint(<expr>))")("expr", right).render();
	else if (!expOrMod || leftSign == Sign::Unsigned)
		right = leftSignString + "(" + right + ")";

	return '(' + left + op + right + ')';
}

string ProtoConverter::visit(Expression const& _expr)
{
	switch (_expr.expr_oneof_case())
	{
	case Expression::kV:
	{
		if (varAvailable())
		{
			string v = visit(_expr.v());
			if (!m_exprSignMap.count(&_expr))
				m_exprSignMap.emplace(&_expr, m_varTypeMap[v]);
			return v;
		}
		else
		{
			if (!m_exprSignMap.count(&_expr))
				m_exprSignMap.emplace(&_expr, pair(Sign::Unsigned, "uint"));
			return maskUnsignedToHex(64);
		}
	}
	case Expression::kBop:
	{
		string b = visit(_expr.bop());
		solAssert(m_exprSignMap.count(&_expr.bop().left()), "Sol arith fuzzer: Invalid binop visit");
		if (!m_exprSignMap.count(&_expr))
			m_exprSignMap.emplace(&_expr, m_exprSignMap[&_expr.bop().left()]);
		return b;
	}
	case Expression::EXPR_ONEOF_NOT_SET:
	{
		if (!m_exprSignMap.count(&_expr))
			m_exprSignMap.emplace(&_expr, pair(Sign::Unsigned, "uint"));
		return maskUnsignedToHex(64);
	}
	}
}

string ProtoConverter::visit(VarRef const& _v)
{
	return "v" + std::to_string(_v.id() % m_varCounter);
}

string ProtoConverter::visit(Assignment const& _assignment)
{
	string varName = visit(_assignment.id());
	solAssert(m_varTypeMap.count(varName), "Sol arith fuzzer: Invalid varname");
	Whiskers a(R"(<varName> = <type>(<expr>);)");
	a("varName", varName);
	a("type", m_varTypeMap[varName].second);
	a("expr", visit(_assignment.value()));
	return "\t\t" + a.render() + '\n';
}