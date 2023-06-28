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
#include <libsolidity/experimental/analysis/TypeCheck.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>
#include <libsolidity/ast/AST.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/Visitor.h>
#include <range/v3/view/map.hpp>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

using namespace std;
using namespace solidity;
using namespace langutil;
using namespace solidity::frontend::experimental;

namespace
{
using Term = std::variant<Application, Lambda, InlineAssembly, VariableDeclaration, Reference, Constant>;


optional<pair<reference_wrapper<Term const>, reference_wrapper<Term const>>> destPair(Term const& _term)
{
	if (auto const* app = get_if<Application>(&_term))
		if (auto* nestedApp = get_if<Application>(app->expression.get()))
			if (auto* constant = get_if<Constant>(nestedApp->expression.get()))
				if (constant->name == variant<string, BuiltinConstant>{BuiltinConstant::Pair})
					return std::make_pair(ref(*nestedApp->argument), ref(*app->argument));
	return nullopt;
}

void destTuple(Term const& _term, list<reference_wrapper<Term const>>& _components)
{
	list<reference_wrapper<Term const>> components;
	if (auto const* app = get_if<Application>(&_term))
		if (auto* nestedApp = get_if<Application>(app->expression.get()))
			if (auto* constant = get_if<Constant>(nestedApp->expression.get()))
				if (constant->name == variant<string, BuiltinConstant>{BuiltinConstant::Pair})
				{
					_components.emplace_back(*nestedApp->argument);
					destTuple(*app->argument, _components);
					return;
				}
	_components.emplace_back(_term);
	return;
}

Type type(Term const& _term)
{
	return std::visit([](auto& term) { return term.type; }, _term);
}

void setType(Term& _term, Type _type)
{
	std::visit([&](auto& term) { term.type = _type; }, _term);
}

string termPrinter(AST& _ast, Term const& _term, TypeEnvironment* _env = nullptr, bool _sugarPairs = true, bool _sugarConsts = true, size_t _indent = 0)
{
	auto recurse = [&](Term const& _next) { return termPrinter(_ast, _next, _env, _sugarPairs, _sugarConsts, _indent); };
	static const std::map<BuiltinConstant, const char*> builtinConstants = {
		{BuiltinConstant::Unit, "()"},
		{BuiltinConstant::Pair, "Pair"},
		{BuiltinConstant::Fun, "Fun"},
		{BuiltinConstant::Constrain, "Constrain"},
		{BuiltinConstant::Return, "Return"},
		{BuiltinConstant::Block, "Block"},
		{BuiltinConstant::Statement, "Statement"},
		{BuiltinConstant::ChainStatements, "ChainStatements"},
		{BuiltinConstant::Assign, "Assign"},
		{BuiltinConstant::MemberAccess, "MemberAccess"},
		{BuiltinConstant::Mul, "Mul"},
		{BuiltinConstant::Add, "Add"},
		{BuiltinConstant::Void, "void"},
		{BuiltinConstant::Word, "word"},
		{BuiltinConstant::Integer, "Integer"},
		{BuiltinConstant::Bool, "Bool"},
		{BuiltinConstant::Undefined, "Undefined"},
		{BuiltinConstant::Equal, "Equal"}
	};
	string result = std::visit(util::GenericVisitor{
		[&](Application const& _app) {
			if (_sugarPairs)
				if (auto* nestedApp = get_if<Application>(_app.expression.get()))
					if (auto* constant = get_if<Constant>(nestedApp->expression.get()))
						if (constant->name == variant<string, BuiltinConstant>{BuiltinConstant::Pair})
						{
							list<reference_wrapper<Term const>> components;
							destTuple(_term, components);
							std::string result = "(";
							result += termPrinter(_ast, components.front(), _env);
							components.pop_front();
							for (auto const& component: components)
							{
								result += ", ";
								result += recurse(component);
							}
							result += ")";
							return result;
						}

			if (_sugarConsts)
				if (auto* constant = get_if<Constant>(_app.expression.get()))
					if (auto* builtin = get_if<BuiltinConstant>(&constant->name))
						switch (*builtin)
						{
						case BuiltinConstant::Assign:
							if (auto pair = destPair(*_app.argument))
								return recurse(pair->first) + " = " + recurse(pair->second);
							break;
						case BuiltinConstant::Mul:
							if (auto pair = destPair(*_app.argument))
								return recurse(pair->first) + " * " + recurse(pair->second);
							break;
						case BuiltinConstant::Add:
							if (auto pair = destPair(*_app.argument))
								return recurse(pair->first) + " + " + recurse(pair->second);
							break;
						case BuiltinConstant::Constrain:
							if (auto pair = destPair(*_app.argument))
								return recurse(pair->first) + ":" + recurse(pair->second);
							break;
						case BuiltinConstant::MemberAccess:
							if (auto pair = destPair(*_app.argument))
								return recurse(pair->first) + "." + recurse(pair->second);
							break;
						case BuiltinConstant::Block:
						{
							string result = "{\n";
							_indent++;
							result += std::string(_indent, '\t');
							result += recurse(*_app.argument);
							_indent--;
							result += "\n" + std::string(_indent, '\t') + "}";
							return result;
						}
						case BuiltinConstant::ChainStatements:
							if (auto pair = destPair(*_app.argument))
								return recurse(pair->first) + "\n" + std::string(_indent, '\t') + recurse(pair->second);
							break;
						case BuiltinConstant::Statement:
							return recurse(*_app.argument) + ";";
						default:
							break;
						}

			return recurse(*_app.expression) + "(" + recurse(*_app.argument) + ")";
		},
		[&](Lambda const& _lambda) {
			return "(" + recurse(*_lambda.argument) + " -> " + recurse(*_lambda.value) + ")";
		},
		[&](InlineAssembly const&) -> string {
			return "assembly";
		},
		[&](VariableDeclaration const& _varDecl) {
			return "let " + recurse(*_varDecl.namePattern) + (_varDecl.initialValue ? " = " + recurse(*_varDecl.initialValue) : "");
		},
		[&](Reference const& _reference) {
		  return "" + _ast.declarations.at(_reference.index).name + "";
		},
		[&](Constant const& _constant) {
			return "" + std::visit(util::GenericVisitor{
				[](BuiltinConstant _constant) -> string {
					return builtinConstants.at(_constant);
				},
				[](std::string const& _name) {
					return _name;
				}
			}, _constant.name) + "";
		}
	}, _term);
	if (_env)
	{
		Type termType = type(_term);
		if (!holds_alternative<std::monostate>(termType))
		{
			result += "[:" + TypeEnvironmentHelpers{*_env}.typeToString(termType) + "]";
		}
	}
	return result;
}

std::string functionPrinter(AST& _ast, AST::FunctionInfo const& _info, TypeEnvironment* _env = nullptr, bool _sugarPairs = true, bool _sugarConsts = true, size_t _indent = 0)
{
	auto printTerm = [&](Term const& _term) { return termPrinter(_ast, _term, _env, _sugarPairs, _sugarConsts, _indent + 1); };
	return "function (" + printTerm(*_info.arguments) + ") -> " + printTerm(*_info.returnType) + " = " + printTerm(*_info.function) + "\n";
}

std::string astPrinter(AST& _ast, TypeEnvironment* _env = nullptr, bool _sugarPairs = true, bool _sugarConsts = true, size_t _indent = 0)
{
	auto printTerm = [&](Term const& _term) { return termPrinter(_ast, _term, _env, _sugarPairs, _sugarConsts, _indent + 1); };
	auto printFunction = [&](AST::FunctionInfo const& _info) { return functionPrinter(_ast, _info, _env, _sugarPairs, _sugarConsts, _indent + 1); };
	std::string result;
	for (auto& info: _ast.typeDefinitions | ranges::view::values)
	{
		result += "type " + printTerm(*info.declaration);
		if (info.arguments)
			result += " " + printTerm(*info.arguments);
		if (info.value)
			result += " = " + printTerm(*info.declaration);
		result += "\n\n";
	}
	for (auto& info: _ast.typeClasses | ranges::view::values)
	{
		result += "class " + printTerm(*info.typeVariable) + ":" + printTerm(*info.declaration) + " {";
		_indent++;
		for (auto&& functionInfo: info.functions | ranges::view::values)
			result += printFunction(functionInfo);
		_indent--;
		result += "}\n\n";
	}
	for (auto& info: _ast.typeClassInstantiations | ranges::view::values)
	{
		result += "instantiation " + printTerm(*info.typeConstructor) + "(" + printTerm(*info.argumentSorts) + "):" + printTerm(*info.typeClass) + "{\n";
		_indent++;
		for (auto&& functionInfo: info.functions | ranges::view::values)
			result += printFunction(functionInfo);
		_indent--;
	}
	for (auto& functionInfo: _ast.functions | ranges::views::values)
	{
		result += printFunction(functionInfo);
		result += "\n";
	}
	for (auto& [contract, info]: _ast.contracts)
	{
		result += "contract " + contract->name() + " {\n";
		_indent++;
		for(auto& function: info.functions | ranges::view::values)
			result += printFunction(function);
		_indent--;
		result += "}\n\n";

	}
	return result;
}

}

namespace
{
struct TVar
{
	TypeEnvironment& env;
	Type type;
};
inline TVar operator>>(TVar a, TVar b)
{
	TypeSystemHelpers helper{a.env.typeSystem()};
	return TVar{a.env, helper.functionType(a.type, b.type)};
}
inline TVar operator,(TVar a, TVar b)
{
	TypeSystemHelpers helper{a.env.typeSystem()};
	return TVar{a.env, helper.tupleType({a.type, b.type})};
}
template <typename T>
struct ArgumentCount;
template <typename R, typename... Args>
struct ArgumentCount<std::function<R(Args...)>> {
	static constexpr size_t value = sizeof...(Args);
};
struct TypeGenerator
{
	template<typename Generator>
	TypeGenerator(Generator&& _generator):generator([generator = std::move(_generator)](TypeEnvironment& _env) -> Type {
		return invoke(_env, generator, std::make_index_sequence<ArgumentCount<decltype(std::function{_generator})>::value>{});
	}) {}
	TypeGenerator(TVar _type):generator([type = _type.type](TypeEnvironment& _env) -> Type { return _env.fresh(type); }) {}
	TypeGenerator(PrimitiveType _type): generator([type = _type](TypeEnvironment& _env) -> Type { return _env.typeSystem().type(type, {}); }) {}
	Type operator()(TypeEnvironment& _env) const { return generator(_env); }
private:
	template<size_t I>
	static TVar makeFreshVariable(TypeEnvironment& _env) { return TVar{_env, _env.typeSystem().freshTypeVariable({}) }; }
	template<typename Generator, size_t... Is>
	static Type invoke(TypeEnvironment& _env, Generator&& _generator, std::index_sequence<Is...>)
	{
		// Use an auxiliary array to ensure deterministic evaluation order.
		std::array<TVar, sizeof...(Is)> tvars{makeFreshVariable<Is>(_env)...};
		return std::invoke(_generator, tvars[Is]...).type;
	}
	std::function<Type(TypeEnvironment&)> generator;
};
}

void TypeCheck::operator()(AST& _ast)
{
	TypeSystem& typeSystem = m_analysis.typeSystem();
	TypeSystemHelpers helper{typeSystem};
	TypeEnvironment& env = typeSystem.env();

	TVar unit = TVar{env, typeSystem.type(PrimitiveType::Unit, {})};
	TVar word = TVar{env, typeSystem.type(PrimitiveType::Word, {})};
	std::unique_ptr<TVar> currentReturn;
	std::map<BuiltinConstant, TypeGenerator> builtinConstantTypeGenerators{
		{BuiltinConstant::Unit, unit},
		{BuiltinConstant::Pair, [](TVar a, TVar b) { return a >> (b >> (a,b)); }},
		{BuiltinConstant::Word, word},
		{BuiltinConstant::Assign, [=](TVar a) { return (a,a) >> unit; }}, // TODO: (a,a) >> a
		{BuiltinConstant::Block, [](TVar a) { return a >> a; }},
		{BuiltinConstant::ChainStatements, [](TVar a, TVar b) { return (a,b) >> b; }},
		{BuiltinConstant::Statement, [=](TVar a) { return a >> unit; }},
		{BuiltinConstant::Return, [&]() {
			 solAssert(currentReturn);
			 return *currentReturn >> unit;
		 }},
		{BuiltinConstant::Fun, [&](TVar a, TVar b) {
			 return (a,b) >> (a >> b);
		 }},
	};

	auto unifyForTerm = [&](Type _a, Type _b, Term* _term) {
		for (auto failure: env.unify(_a, _b))
		{
			TypeEnvironmentHelpers envHelper{env};
			SourceLocation _location = _term ? locationOf(*_term) : SourceLocation{};
			std::visit(util::GenericVisitor{
				[&](TypeEnvironment::TypeMismatch _typeMismatch) {
					m_analysis.errorReporter().typeError(
						0000_error,
						_location,
						fmt::format(
							"Cannot unify {} and {}.",
							envHelper.typeToString(_typeMismatch.a),
							envHelper.typeToString(_typeMismatch.b))
					);
				},
				[&](TypeEnvironment::SortMismatch _sortMismatch) {
					m_analysis.errorReporter().typeError(0000_error, _location, fmt::format(
						"{} does not have sort {}",
						envHelper.typeToString(_sortMismatch.type),
						TypeSystemHelpers{m_analysis.typeSystem()}.sortToString(_sortMismatch.sort)
					));
				},
				[&](TypeEnvironment::RecursiveUnification _recursiveUnification) {
					m_analysis.errorReporter().typeError(
						0000_error,
						_location,
						fmt::format(
							"Recursive unification: {} occurs in {}.",
							envHelper.typeToString(_recursiveUnification.var),
							envHelper.typeToString(_recursiveUnification.type))
					);
				}
			}, failure);
		}
	};
	auto checkTerm = [&](Term& _root) {
		std::list<reference_wrapper<Term>> heap;
		heap.emplace_back(_root);
		auto checked = [](Term const& _term) {
			return !holds_alternative<std::monostate>(type(_term));
		};
		auto canCheck = [&](Term& _term) -> bool {
			bool hasUnchecked = false;
			auto stage = [&](Term& _term) {
				if (!checked(_term))
				{
					heap.push_back(_term);
					hasUnchecked = true;
				}
			};
			std::visit(util::GenericVisitor{
				[&](Application const& _app) {
					stage(*_app.expression);
					stage(*_app.argument);
				},
				[&](Lambda const& _lambda)
				{
					stage(*_lambda.argument);
					stage(*_lambda.value);
				},
				[&](InlineAssembly const&)
				{
					// TODO
				},
				[&](VariableDeclaration const& _varDecl)
				{
					stage(*_varDecl.namePattern);
					if (_varDecl.initialValue)
						stage(*_varDecl.initialValue);
				},
				[&](Reference const&)
				{
				},
				[&](Constant const&) {}
			}, _term);
			if (hasUnchecked)
			{
				stage(_term);
				return false;
			}
			return true;
		};
		std::map<size_t, Type> declarationTypes;
		while (!heap.empty())
		{
			Term& current = heap.front();
			heap.pop_front();
			if (checked(current))
				continue;
			if (!canCheck(current))
				continue;

			auto unify = [&](Type _a, Type _b) { unifyForTerm(_a, _b, &current); };

			std::visit(util::GenericVisitor{
				[&](Application const& _app) {
					if (auto* constant = get_if<Constant>(_app.expression.get()))
						if (auto* builtin = get_if<BuiltinConstant>(&constant->name))
							if (*builtin == BuiltinConstant::Constrain)
								if (auto args = destPair(*_app.argument))
								{
									Type result = type(args->first);
									unify(result, type(args->second));
									setType(current, result);
									return;
								}
					Type resultType = typeSystem.freshTypeVariable({});
					unify(helper.functionType(type(*_app.argument), resultType), type(*_app.expression));
					setType(current, resultType);
				},
				[&](Lambda const& _lambda)
				{
					setType(current, helper.functionType(type(*_lambda.argument), type(*_lambda.value)));
				},
				[&](InlineAssembly const& _inlineAssembly)
				{
					// TODO
					(void)_inlineAssembly;
					setType(current, typeSystem.type(PrimitiveType::Unit, {}));
				},
				[&](VariableDeclaration const& _varDecl)
				{
					Type name = type(*_varDecl.namePattern);
					if (_varDecl.initialValue)
						unify(name, type(*_varDecl.initialValue));
					setType(current, name);
				},
				[&](Reference const& _reference)
				{
					Type result = typeSystem.freshTypeVariable({});
					if (
						auto [it, newlyInserted] = declarationTypes.emplace(_reference.index, result);
						!newlyInserted
					)
						unify(result, it->second);
					setType(current, result);
				},
				[&](Constant const& _constant)
				{
					bool assigned = std::visit(util::GenericVisitor{
						[&](std::string const&) { return false; },
						[&](BuiltinConstant const& _constant) {
							if (auto* generator = util::valueOrNullptr(builtinConstantTypeGenerators, _constant))
							{
								setType(current, (*generator)(env));
								return true;
							}
							return false;
						}
					}, _constant.name);
					if (!assigned)
						setType(current, typeSystem.freshTypeVariable({}));
				}
			}, current);
			solAssert(checked(current));
			if (auto declaration = termBase(current).declaration)
			{
				if (
					auto [it, newlyInserted] = declarationTypes.emplace(*declaration, type(current));
					!newlyInserted
				)
					unify(type(current), it->second);
			}
		}
	};
	for(auto& info: _ast.typeDefinitions | ranges::view::values)
	{
		if (info.arguments)
			checkTerm(*info.arguments);
		if (info.value)
			checkTerm(*info.value);
		checkTerm(*info.declaration);
	}
	for(auto& info: _ast.contracts | ranges::view::values)
		for(auto& function: info.functions | ranges::view::values)
		{
			checkTerm(*function.returnType);
			ScopedSaveAndRestore returnType{currentReturn, std::make_unique<TVar>(TVar{env,type(*function.returnType)})};
			checkTerm(*function.function);
			checkTerm(*function.arguments);
			// TODO: unify stuff?

		}
	for(auto&& info: _ast.functions | ranges::view::values)
	{
		checkTerm(*info.returnType);
		ScopedSaveAndRestore returnType{currentReturn, std::make_unique<TVar>(TVar{env,type(*info.returnType)})};
		checkTerm(*info.function);
		checkTerm(*info.arguments);
		// TODO: unify stuff
	}

	std::cout << astPrinter(_ast, &env) << std::endl;
}