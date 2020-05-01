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
	blockStr << visit(_block.v());
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
		return visit(_stmt.a());
	case Statement::kU:
		return visit(_stmt.u());
	case Statement::STMT_ONEOF_NOT_SET:
		return "";
	}
}

string ProtoConverter::visit(VarDecl const& _vardecl)
{
	bool varExists = varAvailable();
	Whiskers v(R"(<type> <varName> = <type>(<value>);)");
	string type = visit(_vardecl.t());
	string varName = newVarName();
	m_varTypeMap.emplace(varName, type);
	v("type", type);
	v("varName", varName);
	v("value", varExists ? visit(_vardecl.value()) : to_string((*m_rand)()));
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
//	case BinaryOp_Op_ADDSELF:
//		op = " += ";
//		break;
//	case BinaryOp_Op_SUBSELF:
//		op = " -= ";
//		break;
//	case BinaryOp_Op_MULSELF:
//		op = " *= ";
//		break;
//	case BinaryOp_Op_DIVSELF:
//		op = " /= ";
//		break;
//	case BinaryOp_Op_MODSELF:
//		op = " %= ";
//		break;
	case BinaryOp_Op_EXP:
		op = " ** ";
		break;
	case BinaryOp_Op_SHL:
		op = " << ";
		break;
	case BinaryOp_Op_SHR:
		op = " >> ";
		break;
//	case BinaryOp_Op_SHLSELF:
//		op = " <<= ";
//		break;
//	case BinaryOp_Op_SHRSELF:
//		op = " >>= ";
//		break;
	}
	return visit(_bop.left()) + op + visit(_bop.right());
}

string ProtoConverter::visit(Expression const& _expr)
{
	switch (_expr.expr_oneof_case())
	{
	case Expression::kV:
	{
		solAssert(varAvailable(), "Sol arith fuzzer: Varref unavaileble");
		string v = visit(_expr.v());
		if (!m_exprSignMap.count(&_expr))
			m_exprSignMap.emplace(&_expr, m_varTypeMap[v].first);
	}
	case Expression::kBop:
	{
		string b = visit(_expr.bop());
		if (!m_exprSignMap.count(&_expr))
			m_exprSignMap.emplace(&_expr, m_varTypeMap[v].first);
	}
	case Expression::EXPR_ONEOF_NOT_SET:
		return "v0";
	}
}

string ProtoConverter::visit(VarRef const&)
{
	return randomVarName();
}

string ProtoConverter::visit(Assignment const& _assignment)
{
	if (varAvailable())
	{
		string varName = visit(_assignment.id());
		solAssert(m_varTypeMap.count(varName), "Sol arith fuzzer: Invalid varname");
		Whiskers a(R"(<varName> = <type>(<expr>);)");
		a("varName", varName);
		a("type", m_varTypeMap[varName]);
		a("expr", visit(_assignment.value()));
		return "\t\t" + a.render() + '\n';
	}
	else
		return "";
}