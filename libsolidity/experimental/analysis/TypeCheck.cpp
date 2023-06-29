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

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/Visitor.h>

#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>

using namespace std;
using namespace solidity;
using namespace langutil;
using namespace solidity::frontend::experimental;

namespace
{

struct TPat
{
	using Unifier = std::function<void(Type, Type)>;
	template<typename R, typename... Args>
	TPat(R _f(Args...)): generator([f = _f](TypeEnvironment& _env, Unifier _unifier) -> Type {
		return invoke(_env, _unifier, f, std::make_index_sequence<sizeof...(Args)>{});
	}) {}
	TPat(PrimitiveType _type): generator([type = _type](TypeEnvironment& _env, Unifier) { return _env.typeSystem().type(type, {}); }) {}
	Type realize(TypeEnvironment& _env, Unifier _unifier) const { return generator(_env, _unifier); }
	TPat(std::function<Type(TypeEnvironment&, Unifier)> _generator):generator(std::move(_generator)) {}
	TPat(Type _t): generator([t = _t](TypeEnvironment&, Unifier) -> Type { return t; }) {}
private:
	template<size_t I>
	static TPat makeFreshVariable(TypeEnvironment& _env) { return TPat{_env.typeSystem().freshTypeVariable({})}; }
	template<typename Generator, size_t... Is>
	static Type invoke(TypeEnvironment& _env, Unifier _unifier, Generator const& _generator, std::index_sequence<Is...>)
	{
		// Use an auxiliary array to ensure deterministic evaluation order.
		[[maybe_unused]] std::array<TPat, sizeof...(Is)> patterns{makeFreshVariable<Is>(_env)...};
		return (_generator(std::move(patterns[Is])...)).realize(_env, _unifier);
	}
	std::function<Type(TypeEnvironment&, Unifier)> generator;
};
namespace pattern_ops
{
using Unifier = std::function<void(Type, Type)>;
inline TPat operator>>(TPat _a, TPat _b)
{
	return TPat([a = std::move(_a), b = std::move(_b)](TypeEnvironment& _env, Unifier _unifier) -> Type {
		return TypeSystemHelpers{_env.typeSystem()}.functionType(a.realize(_env, _unifier), b.realize(_env, _unifier));
	});
}
inline TPat operator==(TPat _a, TPat _b)
{
	return TPat([a = std::move(_a), b = std::move(_b)](TypeEnvironment& _env, Unifier _unifier) -> Type {
		Type left = a.realize(_env, _unifier);
		Type right = b.realize(_env, _unifier);
		_unifier(left, right);
		return left;
	});
}
template<typename... Args>
TPat tuple(Args... args)
{
	return TPat([args = std::array<TPat, sizeof...(Args)>{{std::move(args)...}}](TypeEnvironment& _env, Unifier _unifier) -> Type {
		return TypeSystemHelpers{_env.typeSystem()}.tupleType(
			args | ranges::view::transform([&](TPat _pat) { return _pat.realize(_env, _unifier); }) | ranges::to<vector<Type>>
		);
	});
}
}

struct BuiltinConstantInfo
{
	std::string name;
	std::optional<TPat> builtinType;
};
[[maybe_unused]] BuiltinConstantInfo const& builtinConstantInfo(BuiltinConstant _constant)
{
	using namespace pattern_ops;
	static const TPat unit{PrimitiveType::Unit};
	static const auto info = std::map<BuiltinConstant, BuiltinConstantInfo>{
		{BuiltinConstant::Unit, {"Unit", unit}},
		{BuiltinConstant::Pair, {"Pair", +[](TPat a, TPat b) { return a >> (b >> tuple(a,b)); }}},
		{BuiltinConstant::Fun, {"Fun", +[](TPat a, TPat b) { return tuple(a,b) >> (a >> b); }}},
		{BuiltinConstant::Constrain, {"Constrain", +[](TPat a) { return tuple(a,a) >> a; }}},
		{BuiltinConstant::NamedTerm, {"NamedTerm", +[](TPat a) { return tuple(unit, a) >> a; /* TODO: (name, a) >> a */ }}},
		{BuiltinConstant::TypeDeclaration, {"TypeDeclaration", nullopt}},
		{BuiltinConstant::TypeDefinition, {"TypeDefinition", +[](TPat type, TPat args, TPat value) {
			return tuple(type, args, value) >> (args >> type);
		}}},
		{BuiltinConstant::TypeClassDefinition, {"TypeClassDefinition", nullopt}},
		{BuiltinConstant::TypeClassInstantiation, {"TypeClassInstantiation", nullopt}},
		{BuiltinConstant::FunctionDeclaration, {"FunctionDeclaration", nullopt}},
		{BuiltinConstant::FunctionDefinition, {"FunctionDefinition", +[](TPat a, TPat r) {
			return tuple(a >> r, a, r, r) >> (a >> r);
		}}},
		{BuiltinConstant::ContractDefinition, {"ContractDefinition", +[]() {
			return tuple(unit, (unit >> unit)) >> unit;
		}}},
		{BuiltinConstant::VariableDeclaration, {"VariableDeclaration", +[](TPat a) { return a >> a; }}},
		{BuiltinConstant::VariableDefinition, {"VariableDefinition", nullopt}},
		{BuiltinConstant::Block, {"Block", +[](TPat a) { return a >> a; }}},
		{BuiltinConstant::ReturnStatement, {"ReturnStatement", +[](TPat a) { return a >> a; }}},
		{BuiltinConstant::RegularStatement, {"RegularStatement", +[](TPat a) { return a >> unit; }}},
		{BuiltinConstant::ChainStatements, {"ChainStatements", +[](TPat a, TPat b) { return tuple(a,b) >> b; }}},
		{BuiltinConstant::Assign, {"Assign", +[](TPat a) { return tuple(a,a) >> unit; }}},
		{BuiltinConstant::MemberAccess, {"MemberAccess", nullopt}},
		{BuiltinConstant::Mul, {"Mul", nullopt}},
		{BuiltinConstant::Add, {"Add", nullopt}},
		{BuiltinConstant::Void, {"Void", nullopt}},
		{BuiltinConstant::Word, {"Word", PrimitiveType::Word}},
		{BuiltinConstant::Integer, {"Integer", nullopt}},
		{BuiltinConstant::Bool, {"Bool", nullopt}},
		{BuiltinConstant::Undefined, {"Undefined", nullopt}},
		{BuiltinConstant::Equal, {"Equal", nullopt}},
	};
	return info.at(_constant);
}

template<typename Visitor>
void forEachTopLevelTerm(AST& _ast, Visitor _visitor)
{
	for (auto& term: _ast.typeDefinitions | ranges::view::values)
		_visitor(*term);
	for (auto& term: _ast.typeClasses | ranges::view::values)
		_visitor(*term);
	for (auto& term: _ast.typeClassInstantiations | ranges::view::values)
		_visitor(*term);
	for (auto& term: _ast.functions | ranges::views::values)
		_visitor(*term);
	for (auto& term: _ast.contracts | ranges::views::values)
		_visitor(*term);
}

template<typename Visitor>
void forEachImmediateSubTerm(Term& _term, Visitor _visitor)
{
	std::visit(util::GenericVisitor{
		[&](Application const& _app) {
			_visitor(*_app.expression);
			_visitor(*_app.argument);
		},
		[&](Lambda const& _lambda)
		{
			_visitor(*_lambda.argument);
			_visitor(*_lambda.value);
		},
		[&](InlineAssembly const&)
		{
			// TODO
		},
		[&](Reference const&) {},
		[&](Constant const&) {}
	}, _term);
}

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

string colorize(string _color, string _string)
{
	return _color + _string + util::formatting::RESET;
}

string termPrinter(AST& _ast, Term const& _term, TypeEnvironment* _env = nullptr, bool _sugarPairs = true, bool _sugarConsts = true, size_t _indent = 0)
{
	using namespace util::formatting;
	auto recurse = [&](Term const& _next) { return termPrinter(_ast, _next, _env, _sugarPairs, _sugarConsts, _indent); };
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
						case BuiltinConstant::RegularStatement:
							return recurse(*_app.argument) + ";";
						case BuiltinConstant::ReturnStatement:
							return colorize(CYAN, "return ") + recurse(*_app.argument) + ";";
						default:
							break;
						}

			return recurse(*_app.expression) + "(" + recurse(*_app.argument) + ")";
		},
		[&](Lambda const& _lambda) {
			return "(" + recurse(*_lambda.argument) + " -> " + recurse(*_lambda.value) + ")";
		},
		[&](InlineAssembly const&) -> string {
			return colorize(CYAN, "assembly");
		},
		[&](Reference const& _reference) {
		  return _reference.name.empty() ? util::toString(_reference.index) : _reference.name;
		},
		[&](Constant const& _constant) {
			return colorize(BLUE, std::visit(util::GenericVisitor{
				[](BuiltinConstant _constant) -> string {
					return builtinConstantInfo(_constant).name;
				},
				[](std::string const& _name) {
					return _name;
				}
			}, _constant.name));
		}
	}, _term);
	if (_env)
	{
		Type termType = type(_term);
		if (!holds_alternative<std::monostate>(termType))
		{
			result += colorize(GREEN, "[:" + TypeEnvironmentHelpers{*_env}.typeToString(termType) + "]");
		}
	}
	return result;
}

std::string astPrinter(AST& _ast, TypeEnvironment* _env = nullptr, bool _sugarPairs = true, bool _sugarConsts = true, size_t _indent = 0)
{
	std::string result;
	auto printTerm = [&](Term const& _term) { result += termPrinter(_ast, _term, _env, _sugarPairs, _sugarConsts, _indent + 1) + "\n\n"; };
	forEachTopLevelTerm(_ast, printTerm);
	return result;
}

}

void TypeCheck::operator()(AST& _ast)
{
	TypeSystem& typeSystem = m_analysis.typeSystem();
	TypeSystemHelpers helper{typeSystem};
	TypeEnvironment& env = typeSystem.env();

	list<reference_wrapper<Term>> toCheck;
	forEachTopLevelTerm(_ast, [&](Term& _root) {
		list<reference_wrapper<Term>> staged{{_root}};
		while (!staged.empty())
		{
			Term& term = staged.front().get();
			staged.pop_front();
			toCheck.push_back(term);
			forEachImmediateSubTerm(term, [&](Term& _subTerm) { staged.push_back(_subTerm); });
		}
	});

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

	std::map<size_t, Type> declarationTypes;
	for(auto term: toCheck | ranges::view::reverse)
	{
		auto unify = [&](Type _a, Type _b) { unifyForTerm(_a, _b, &term.get()); };
		std::visit(util::GenericVisitor{
			[&](Application const& _app) {
				/*if (auto* constant = get_if<Constant>(_app.expression.get()))
					if (auto* builtin = get_if<BuiltinConstant>(&constant->name))
						if (*builtin == BuiltinConstant::Constrain)
							if (auto args = destPair(*_app.argument))
							{
								Type result = type(args->first);
								unify(result, type(args->second));
								setType(term, result);
								return;
							}*/
				Type resultType = typeSystem.freshTypeVariable({});
				unify(helper.functionType(type(*_app.argument), resultType), type(*_app.expression));
				setType(term, resultType);
			},
			[&](Lambda const& _lambda)
			{
				setType(term, helper.functionType(type(*_lambda.argument), type(*_lambda.value)));
			},
			[&](InlineAssembly const& _inlineAssembly)
			{
				// TODO
				(void)_inlineAssembly;
				setType(term, typeSystem.type(PrimitiveType::Unit, {}));
			},
			[&](Reference const& _reference)
			{
				Type result = typeSystem.freshTypeVariable({});
				if (
					auto [it, newlyInserted] = declarationTypes.emplace(_reference.index, result);
					!newlyInserted
				)
					unify(result, it->second);
				setType(term, result);
			},
			[&](Constant const& _constant)
			{
				bool assigned = std::visit(util::GenericVisitor{
					[&](std::string const&) { return false; },
					[&](BuiltinConstant const& _constant) {
						if (auto generator = builtinConstantInfo(_constant).builtinType)
						{
							setType(term, (*generator).realize(env, unify));
							return true;
						}
						return false;
					}
				}, _constant.name);
				if (!assigned)
					setType(term, typeSystem.freshTypeVariable({}));
			}
		}, term.get());
		solAssert(!holds_alternative<std::monostate>(type(term)));
	}

	std::cout << astPrinter(_ast, &env) << std::endl;
}