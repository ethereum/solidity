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
/**
 * Analysis part of inline assembly.
 */

#pragma once

#include <libsolidity/interface/Exceptions.h>

#include <boost/variant.hpp>

#include <functional>
#include <memory>

namespace dev
{
namespace solidity
{
namespace assembly
{

struct Literal;
struct Block;
struct Label;
struct FunctionalInstruction;
struct FunctionalAssignment;
struct VariableDeclaration;
struct Instruction;
struct Identifier;
struct Assignment;
struct FunctionDefinition;
struct FunctionCall;

template <class...>
struct GenericVisitor{};

template <class Visitable, class... Others>
struct GenericVisitor<Visitable, Others...>: public GenericVisitor<Others...>
{
	using GenericVisitor<Others...>::operator ();
	explicit GenericVisitor(
		std::function<void(Visitable&)> _visitor,
		std::function<void(Others&)>... _otherVisitors
	):
		GenericVisitor<Others...>(_otherVisitors...),
		m_visitor(_visitor)
	{}

	void operator()(Visitable& _v) const { m_visitor(_v); }

	std::function<void(Visitable&)> m_visitor;
};
template <>
struct GenericVisitor<>: public boost::static_visitor<> {
	void operator()() const {}
};


struct Scope
{
	struct Variable
	{
		int stackHeight = 0;
		bool active = false;
	};

	struct Label
	{
		size_t id = unassignedLabelId;
		static const size_t errorLabelId = -1;
		static const size_t unassignedLabelId = 0;
	};

	struct Function
	{
		Function(size_t _arguments, size_t _returns): arguments(_arguments), returns(_returns) {}
		size_t arguments = 0;
		size_t returns = 0;
	};

	using Identifier = boost::variant<Variable, Label, Function>;
	using Visitor = GenericVisitor<Variable const, Label const, Function const>;
	using NonconstVisitor = GenericVisitor<Variable, Label, Function>;

	bool registerVariable(std::string const& _name);
	bool registerLabel(std::string const& _name);
	bool registerFunction(std::string const& _name, size_t _arguments, size_t _returns);

	/// Looks up the identifier in this or super scopes (stops and function and assembly boundaries)
	/// and returns a valid pointer if found or a nullptr if not found.
	/// The pointer will be invalidated if the scope is modified.
	Identifier* lookup(std::string const& _name);
	/// Looks up the identifier in this and super scopes (stops and function and assembly boundaries)
	/// and calls the visitor, returns false if not found.
	template <class V>
	bool lookup(std::string const& _name, V const& _visitor)
	{
		if (Identifier* id = lookup(_name))
		{
			boost::apply_visitor(_visitor, *id);
			return true;
		}
		else
			return false;
	}
	/// @returns true if the name exists in this scope or in super scopes (also searches
	/// across function and assembly boundaries).
	bool exists(std::string const& _name);
	Scope* superScope = nullptr;
	/// If true, identifiers from the super scope are not visible here, but they are still
	/// taken into account to prevent shadowing.
	bool closedScope = false;
	std::map<std::string, Identifier> identifiers;
};


class AsmAnalyzer: public boost::static_visitor<bool>
{
public:
	using Scopes = std::map<assembly::Block const*, std::shared_ptr<Scope>>;
	AsmAnalyzer(Scopes& _scopes, ErrorList& _errors);

	bool operator()(assembly::Instruction const&) { return true; }
	bool operator()(assembly::Literal const& _literal);
	bool operator()(assembly::Identifier const&) { return true; }
	bool operator()(assembly::FunctionalInstruction const& _functionalInstruction);
	bool operator()(assembly::Label const& _label);
	bool operator()(assembly::Assignment const&) { return true; }
	bool operator()(assembly::FunctionalAssignment const& _functionalAssignment);
	bool operator()(assembly::VariableDeclaration const& _variableDeclaration);
	bool operator()(assembly::FunctionDefinition const& _functionDefinition);
	bool operator()(assembly::FunctionCall const& _functionCall);
	bool operator()(assembly::Block const& _block);

private:
	Scope* m_currentScope = nullptr;
	Scopes& m_scopes;
	ErrorList& m_errors;
};

}
}
}
