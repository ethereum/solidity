#pragma once

#include <libyul/AsmData.h>
#include <liblangutil/ErrorReporter.h>

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace yul
{

class StructualEquality
{
private:
	langutil::ErrorReporter& m_errorReporter;

public:
	explicit StructualEquality(langutil::ErrorReporter _errorReporter): m_errorReporter{_errorReporter} {}

	void compare(Statement const& actual, Statement const& expected);
	void compare(Expression const& actual, Expression const& expected);
	void compare(Expression const* actual, Expression const* expected);

	template<typename T>
	void compare(std::vector<T> const& a, std::vector<T> const& b);

	void compare(Block const& a, Block const& b);
	void compare(Instruction const& a, Instruction const& b);
	void compare(Literal const& a, Literal const& b);
	bool compare(Identifier const& a, Identifier const& b);
	void compare(Label const& a, Label const& b);
	void compare(StackAssignment const& a, StackAssignment const& b);
	void compare(Assignment const& a, Assignment const& b);
	void compare(FunctionalInstruction const& a, FunctionalInstruction const& b);
	void compare(FunctionCall const& a, FunctionCall const& b);
	void compare(ExpressionStatement const& a, ExpressionStatement const& b);
	void compare(VariableDeclaration const& a, VariableDeclaration const& b);
	bool compare(TypedName const& a, TypedName const& b);
	void compare(FunctionDefinition const& a, FunctionDefinition const& b);
	void compare(If const& a, If const& b);
	void compare(Case const& a, Case const& b);
	void compare(Switch const& a, Switch const& b);
	void compare(ForLoop const& a, ForLoop const& b);
	void compare(Break const&, Break const&);
	void compare(Continue const&, Continue const&);

private:
	// a little convenience helper
	template <typename T>
	bool fail(T const* _actual, T const* _expected) {
		m_errorReporter.declarationError(
			locationOf(*_actual),
			langutil::SecondarySourceLocation{}.append("Expected instead:", locationOf(*_expected)),
			"Unexpected syntactical element."
		);
		return false;
	}

	template <typename T>
	void tryCompare(Statement const& a, Statement const& b)
	{
		if (a.type() != typeid(T) || b.type() != typeid(T))
			fail(&a, &b);
		else
			compare(boost::get<T>(a), boost::get<T>(b));
	}

	template <typename T, typename T2, typename... MoreT>
	void tryCompare(Statement const& a, Statement const& b)
	{
		if (a.type() == typeid(T) && b.type() == typeid(T))
			compare(boost::get<T>(a), boost::get<T>(b));
		else
			tryCompare<T2, MoreT...>(a, b);
	}

	template <typename T>
	void tryCompareExpr(Expression const& a, Expression const& b)
	{
		if (a.type() == typeid(T) && b.type() == typeid(T))
			compare(boost::get<T>(a), boost::get<T>(b));
		else
			fail(&a, &b);
	}

	template <typename T, typename T2, typename... MoreT>
	void tryCompareExpr(Expression const& a, Expression const& b)
	{
		if (a.type() == typeid(T) && b.type() == typeid(T))
			compare(boost::get<T>(a), boost::get<T>(b));
		else
			tryCompareExpr<T2, MoreT...>(a, b);
	}

	template <typename T>
	bool leaf(T const& a, T const& b, bool result)
	{
		if (!result)
			return fail(&a, &b);

		return result;
	}
};

} // namespace yul
