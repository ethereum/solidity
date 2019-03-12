#include <libyul/StructualEquality.h>
#include <libyul/AsmPrinter.h>

#include <algorithm>
#include <tuple>

using namespace std;
using namespace yul;

void StructualEquality::compare(Statement const& a, Statement const& b)
{
	tryCompare<
		Assignment, Block, Break, Continue, ExpressionStatement, ForLoop,
		FunctionDefinition, If, Instruction, Label, StackAssignment,
		Switch, VariableDeclaration
	>(a, b);
}

void StructualEquality::compare(Expression const* a, Expression const* b)
{
	if (a && b)
		compare(*a, *b);
	else if (a || b)
		return;// TODO: report invariant
}

void StructualEquality::compare(Expression const& a, Expression const& b)
{
	tryCompareExpr<FunctionalInstruction, FunctionCall, Identifier, Literal>(a, b);
}

template <typename T>
void StructualEquality::compare(vector<T> const& a, vector<T> const& b)
{
	for (size_t i = 0; i < min(a.size(), b.size()); i++)
		compare(a[i], b[i]);

	if (a.size() < b.size())
		return; // TODO fail<T>(nullptr, &b[a.size()]);
	else if (a.size() > b.size())
		return;// TODO fail<T>(&a[b.size()], nullptr);
}

void StructualEquality::compare(Block const& a, Block const& b)
{
	compare(a.statements, b.statements);
}

void StructualEquality::compare(Instruction const& a, Instruction const& b)
{
	leaf(a, b, a.instruction == b.instruction);
}

void StructualEquality::compare(Literal const& a, Literal const& b)
{
	leaf(a, b, a.kind == b.kind && a.value == b.value && a.type == b.type);
}

bool StructualEquality::compare(Identifier const& a, Identifier const& b)
{
	return leaf(a, b, a.name == b.name);
}

void StructualEquality::compare(Label const& a, Label const& b)
{
	leaf(a, b, a.name == b.name);
}

void StructualEquality::compare(StackAssignment const& a, StackAssignment const& b)
{
	compare(a.variableName, b.variableName);
}

void StructualEquality::compare(Assignment const& a, Assignment const& b)
{
	compare(a.variableNames, b.variableNames);
	compare(*a.value, *b.value);
}

void StructualEquality::compare(FunctionalInstruction const& a, FunctionalInstruction const& b)
{
	if (leaf(a, b, a.instruction == b.instruction))
		compare(a.arguments, b.arguments);
}

void StructualEquality::compare(FunctionCall const& a, FunctionCall const& b)
{
	if (compare(a.functionName, b.functionName))
		compare(a.arguments, b.arguments);
}

void StructualEquality::compare(ExpressionStatement const& a, ExpressionStatement const& b)
{
	compare(a.expression, b.expression);
}

void StructualEquality::compare(VariableDeclaration const& a, VariableDeclaration const& b)
{
	compare(a.variables, b.variables);
	compare(a.value.get(), b.value.get());
}

bool StructualEquality::compare(TypedName const& a, TypedName const& b)
{
	return leaf(a, b, a.name == b.name && a.type == b.type);
}

void StructualEquality::compare(FunctionDefinition const& a, FunctionDefinition const& b)
{
	if (leaf(a, b, a.name == b.name))
	{
		compare(a.parameters, b.parameters);
		compare(a.returnVariables, b.returnVariables);
		compare(a.body, b.body);
	}
}

void StructualEquality::compare(If const& a, If const& b)
{
	compare(*a.condition, *b.condition);
	compare(a.body, b.body);
}

void StructualEquality::compare(Case const& a, Case const& b)
{
	if (leaf(a, b, (!a.value.get() && !b.value.get()) || (a.value.get() && b.value.get())))
	{
		if (a.value && b.value)
			compare(*a.value, *b.value);
		compare(a.body, b.body);
	}
}

void StructualEquality::compare(Switch const& a, Switch const& b)
{
	compare(*a.expression, *b.expression);
	compare(a.cases, b.cases);
}

void StructualEquality::compare(ForLoop const& a, ForLoop const& b)
{
	compare(a.pre, b.pre);
	compare(*a.condition, *b.condition);
	compare(a.post, b.post);
	compare(a.body, b.body);
}

void StructualEquality::compare(Break const&, Break const&)
{
}

void StructualEquality::compare(Continue const&, Continue const&)
{
}

