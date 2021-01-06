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
#include <libyul/optimiser/FullSSATransform.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;

void FullSSATransform::run(OptimiserStepContext& _context, Block& _ast)
{
	FullSSATransform transform{_context};
	transform(_ast);
}

FullSSATransform::FullSSATransform(OptimiserStepContext& _context): m_nameDispenser(_context.dispenser)
{
}

void FullSSATransform::operator()(VariableDeclaration& _varDecl)
{
	ASTModifier::operator()(_varDecl);
	for (auto const& _var: _varDecl.variables)
	{
		yulAssert(!m_currentSSANames.count(_var.name), "");
		m_currentSSANames[_var.name] = _var.name;
	}
}

void FullSSATransform::operator()(Identifier& _identifier)
{
	yulAssert(m_currentSSANames.count(_identifier.name), "");
	_identifier.name = m_currentSSANames[_identifier.name];
}

void FullSSATransform::operator()(FunctionDefinition& _functionDefinition)
{
	vector<YulString> oldFunctionReturnVariables;
	swap(oldFunctionReturnVariables, m_currentFunctionReturnVariables);

	for (auto const& var: _functionDefinition.returnVariables)
		m_currentFunctionReturnVariables.emplace_back(var.name);

	for (auto const& argument: _functionDefinition.parameters + _functionDefinition.returnVariables)
	{
		yulAssert(!m_currentSSANames.count(argument.name), "");
		m_currentSSANames[argument.name] = argument.name;
	}

	(*this)(_functionDefinition.body);

	for (auto const& var: _functionDefinition.returnVariables)
		_functionDefinition.body.statements.emplace_back(makePhiStore(var.name, m_currentSSANames[var.name]));

	swap(m_currentFunctionReturnVariables, oldFunctionReturnVariables);
}

Statement FullSSATransform::makePhiStore(YulString _var, YulString _value)
{
	return ExpressionStatement{
		langutil::SourceLocation{}, // TODO
		FunctionCall{
			langutil::SourceLocation{}, // TODO
			Identifier{
				langutil::SourceLocation{},
				YulString{"phi_store"}
			},
			{
				Literal{
					langutil::SourceLocation{}, // TODO
					LiteralKind::String,
					_var,
					YulString{} // TODO
				},
				Identifier{
					langutil::SourceLocation{}, // TODO
					_value
				}
			}
		}
	};
}

Expression FullSSATransform::makePhiLoad(YulString _var)
{
	return FunctionCall{
		langutil::SourceLocation{}, // TODO
		Identifier{
			langutil::SourceLocation{},
			YulString{"phi_load"}
		},
		{
			Literal{
				langutil::SourceLocation{}, // TODO
				LiteralKind::String,
				_var,
				YulString{} // TODO
			}
		}
	};
}

void FullSSATransform::operator()(Block& _block)
{
	auto addReloads = [&](map<YulString, YulString> const& _variables, vector<Statement>& _statements) {
		for (auto [origName, oldName]: _variables)
		{
			YulString newName = m_nameDispenser.newName(origName);
			_statements.emplace_back(
				VariableDeclaration{
					langutil::SourceLocation{},
					{TypedName{langutil::SourceLocation{}, newName, YulString{}}},
					make_unique<Expression>(makePhiLoad(oldName))
				}
			);
			m_currentSSANames[origName] = newName;
		}
	};
	auto saveAssignedVariables = [&](auto const& _names, vector<Statement>& _result) {
		map<YulString, YulString> storedNames;
		for (YulString var: _names)
			if (m_currentSSANames.count(var))
			{
				YulString currentName = m_currentSSANames[var];
				_result.emplace_back(makePhiStore(currentName, currentName));
				storedNames[var] = currentName;
			}
		return storedNames;
	};
	auto restoreAssignedVariables = [&](map<YulString, YulString> const& _storedNames, vector<Statement>& _result) {
		for (auto [origName, oldName]: _storedNames)
		{
			yulAssert(m_currentSSANames.count(origName), "");
			YulString currentName = m_currentSSANames[origName];
			_result.emplace_back(makePhiStore(oldName, currentName));
		}
	};

	using OptionalStatements = optional<vector<Statement>>;
	util::iterateReplacing(
		_block.statements,
		[&](Statement& _stmt) -> OptionalStatements
		{
			return std::visit(util::GenericVisitor{
				[&](Assignment& _assignment) -> OptionalStatements {
					visit(*_assignment.value);
					vector<Statement> result;
					VariableDeclaration varDecl{
						_assignment.location,
						{},
						std::move(_assignment.value)
					};
					for (auto& var: _assignment.variableNames)
					{
						yulAssert(m_currentSSANames.count(var.name), "");
						YulString newName = m_nameDispenser.newName(var.name);
						varDecl.variables.emplace_back(TypedName{
							var.location,
							newName,
							YulString{}
						});
						m_currentSSANames[var.name] = newName;
					}
					result.emplace_back(move(varDecl));
					return result;
				},
				[&](If& _if) -> OptionalStatements {
					Assignments assignments;
					assignments(_if);
					vector<Statement> result;
					auto storedAssignedVariables = saveAssignedVariables(assignments.names(), result);

					(*this)(_if);
					restoreAssignedVariables(storedAssignedVariables, _if.body.statements);
					result.emplace_back(std::move(_if));

					addReloads(storedAssignedVariables, result);
					return result;
				},
				[&](Switch& _switch) -> OptionalStatements {
					Assignments assignments;
					assignments(_switch);
					vector<Statement> result;
					auto storedAssignedVariables = saveAssignedVariables(assignments.names(), result);

					visit(*_switch.expression);
					auto saved = m_currentSSANames;
					for (auto& switchCase: _switch.cases)
					{
						m_currentSSANames = saved;
						if (switchCase.value)
							(*this)(*switchCase.value);
						(*this)(switchCase.body);
						restoreAssignedVariables(storedAssignedVariables, switchCase.body.statements);
					}
					result.emplace_back(std::move(_switch));

					addReloads(storedAssignedVariables, result);
					return result;
				},
				[&](ForLoop& _loop) -> OptionalStatements {
					yulAssert(_loop.pre.statements.empty(), "");
					std::map<YulString, YulString> oldLoopAssignments;
					swap(oldLoopAssignments, m_currentLoopAssignments);
					Assignments assignments;
					assignments(_loop);

					vector<Statement> result;

					if (auto* identifier = std::get_if<Identifier>(_loop.condition.get()))
					{
						if (assignments.names().count(identifier->name))
						{
							yulAssert(m_currentSSANames.count(identifier->name), "");
							_loop.condition = make_unique<Expression>(makePhiLoad(m_currentSSANames[identifier->name]));
						}
					}
					else
						yulAssert(holds_alternative<Literal>(*_loop.condition), "for loop into body required");

					m_currentLoopAssignments = saveAssignedVariables(assignments.names(), result);

					vector<Statement> newBody;
					addReloads(m_currentLoopAssignments, newBody);

					(*this)(_loop.body);

					newBody += std::move(_loop.body.statements);
					_loop.body.statements = std::move(newBody);

					restoreAssignedVariables(m_currentLoopAssignments, _loop.body.statements);

					vector<Statement> newPost;
					addReloads(m_currentLoopAssignments, newPost);

					(*this)(_loop.post);

					restoreAssignedVariables(m_currentLoopAssignments, _loop.post.statements);

					newPost += std::move(_loop.post.statements);
					_loop.post.statements = std::move(newPost);

					result.emplace_back(move(_loop));

					addReloads(m_currentLoopAssignments, result);

					swap(m_currentLoopAssignments, oldLoopAssignments);
					return result;
				},
				[&](Continue& _continue) -> OptionalStatements {
					vector<Statement> result;
					restoreAssignedVariables(m_currentLoopAssignments, result);
					result.emplace_back(std::move(_continue));
					return result;
				},
				[&](Break& _break) -> OptionalStatements {
					vector<Statement> result;
					restoreAssignedVariables(m_currentLoopAssignments, result);
					result.emplace_back(std::move(_break));
					return result;
				},
				[&](Leave& _leaveStatement) -> OptionalStatements {
					vector<Statement> result;
					for (YulString var: m_currentFunctionReturnVariables)
					{
						yulAssert(m_currentSSANames.count(var), "");
						result.emplace_back(makePhiStore(var, m_currentSSANames[var]));
					}
					result.emplace_back(std::move(_leaveStatement));
					return result;
				},
				[&](auto& _stmt) -> OptionalStatements { (*this)(_stmt); return std::nullopt; }
			}, _stmt);
		}
	);
}
