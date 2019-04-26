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

#include <libyul/AsmData.h>
#include <libyul/backends/wasm/WordSizeTransform.h>
#include <libyul/Utilities.h>

#include <libdevcore/CommonData.h>

#include <array>

using namespace std;
using namespace dev;
using namespace yul;

void WordSizeTransform::operator()(FunctionDefinition& _fd)
{
	rewriteVarDeclList(_fd.parameters);
	rewriteVarDeclList(_fd.returnVariables);
	(*this)(_fd.body);
}

void WordSizeTransform::operator()(FunctionalInstruction& _ins)
{
	rewriteFunctionCallArguments(_ins.arguments);
}

void WordSizeTransform::operator()(FunctionCall& _fc)
{
	rewriteFunctionCallArguments(_fc.arguments);
}

void WordSizeTransform::operator()(If&)
{
	yulAssert(false, "If statement not implemented.");
}

void WordSizeTransform::operator()(Switch&)
{
	yulAssert(false, "Switch statement not implemented.");
}

void WordSizeTransform::operator()(Block& _block)
{
	iterateReplacing(
		_block.statements,
		[&](Statement& _s) -> boost::optional<vector<Statement>>
		{
			if (_s.type() == typeid(VariableDeclaration))
			{
				VariableDeclaration& varDecl = boost::get<VariableDeclaration>(_s);
				if (
					!varDecl.value ||
					varDecl.value->type() == typeid(FunctionalInstruction) ||
					varDecl.value->type() == typeid(FunctionCall)
				)
				{
					if (varDecl.value) visit(*varDecl.value);
					rewriteVarDeclList(varDecl.variables);
					return boost::none;
				}
				else if (
					varDecl.value->type() == typeid(Identifier) ||
					varDecl.value->type() == typeid(Literal)
				)
				{
					yulAssert(varDecl.variables.size() == 1, "");
					auto newRhs = expandValue(*varDecl.value);
					auto newLhs = generateU64IdentifierNames(varDecl.variables[0].name);
					vector<Statement> ret;
					for (int i = 0; i < 4; i++)
						ret.push_back(
							VariableDeclaration{
								varDecl.location,
								{TypedName{varDecl.location, newLhs[i], "u64"_yulstring}},
								std::move(newRhs[i])
							}
						);
					return ret;
				}
				else
					yulAssert(false, "");
			}
			else if (_s.type() == typeid(Assignment))
			{
				Assignment& assignment = boost::get<Assignment>(_s);
				yulAssert(assignment.value, "");
				if (
					assignment.value->type() == typeid(FunctionalInstruction) ||
					assignment.value->type() == typeid(FunctionCall)
				)
				{
					if (assignment.value) visit(*assignment.value);
					rewriteIdentifierList(assignment.variableNames);
					return boost::none;
				}
				else if (
					assignment.value->type() == typeid(Identifier) ||
					assignment.value->type() == typeid(Literal)
				)
				{
					yulAssert(assignment.variableNames.size() == 1, "");
					auto newRhs = expandValue(*assignment.value);
					YulString lhsName = assignment.variableNames[0].name;
					vector<Statement> ret;
					for (int i = 0; i < 4; i++)
						ret.push_back(
							Assignment{
								assignment.location,
								{Identifier{assignment.location, m_variableMapping.at(lhsName)[i]}},
								std::move(newRhs[i])
							}
						);
					return ret;
				}
				else
					yulAssert(false, "");
			}
			else
				visit(_s);
			return boost::none;
		}
	);
}

void WordSizeTransform::run(Block& _ast, NameDispenser& _nameDispenser)
{
	WordSizeTransform{_nameDispenser}(_ast);
}

void WordSizeTransform::rewriteVarDeclList(TypedNameList& _nameList)
{
	iterateReplacing(
		_nameList,
		[&](TypedName const& _n) -> boost::optional<TypedNameList>
		{
			TypedNameList ret;
			yulAssert(m_variableMapping.find(_n.name) == m_variableMapping.end(), "");
			for (int i = 0; i < 4; i++)
			{
				auto newName = m_nameDispenser.newName(_n.name);
				m_variableMapping[_n.name][i] = newName;
				ret.push_back(TypedName{_n.location, newName, "u64"_yulstring});
			}
			return ret;
		}
	);
}

void WordSizeTransform::rewriteIdentifierList(vector<Identifier>& _ids)
{
	iterateReplacing(
		_ids,
		[&](Identifier const& _id) -> boost::optional<vector<Identifier>>
		{
			vector<Identifier> ret;
			for (auto newId: m_variableMapping.at(_id.name))
				ret.push_back(Identifier{_id.location, newId});
			return ret;
		}
	);
}

void WordSizeTransform::rewriteFunctionCallArguments(vector<Expression>& _args)
{
	iterateReplacing(
		_args,
		[&](Expression& _e) -> boost::optional<vector<Expression>>
		{
			// ExpressionSplitter guarantees arguments to be Identifier or Literal
			yulAssert(_e.type() == typeid(Identifier) || _e.type() == typeid(Literal), "");
			vector<Expression> ret;
			for (auto& v: expandValue(_e))
				ret.push_back(*v);
			return ret;
		}
	);
}

array<YulString, 4> WordSizeTransform::generateU64IdentifierNames(YulString const& _s)
{
	yulAssert(m_variableMapping.find(_s) == m_variableMapping.end(), "");
	array<YulString, 4> ret;
	for (int i = 0; i < 4; i++)
	{
		auto newName = m_nameDispenser.newName(_s);
		m_variableMapping[_s][i] = newName;
		ret[i] = newName;
	}
	return ret;
}

array<unique_ptr<Expression>, 4> WordSizeTransform::expandValue(Expression const& _e)
{
	array<unique_ptr<Expression>, 4> ret;
	if (_e.type() == typeid(Identifier))
	{
		Identifier const& id = boost::get<Identifier>(_e);
		for (int i = 0; i < 4; i++)
			ret[i] = make_unique<Expression>(Identifier{id.location, m_variableMapping.at(id.name)[i]});
	}
	else if (_e.type() == typeid(Literal))
	{
		Literal const& lit = boost::get<Literal>(_e);
		u256 val = valueOfLiteral(lit);
		for (int i = 3; i >= 0; i--)
		{
			u256 currentVal = val & std::numeric_limits<uint64_t>::max();
			val >>= 64;
			ret[i] = make_unique<Expression>(
				Literal{
					lit.location,
					LiteralKind::Number,
					YulString(currentVal.str()),
					"u64"_yulstring
				}
			);
		}
	}
	else
		yulAssert(false, "");
	return ret;
}

