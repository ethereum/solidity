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

#include <libsolidity/formal/SMTEncoder.h>

#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/formal/SymbolicState.h>
#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/analysis/ConstantEvaluator.h>

#include <libyul/AST.h>
#include <libyul/optimiser/Semantics.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/Helpers.h>

#include <liblangutil/CharStreamProvider.h>

#include <libsolutil/Algorithms.h>

#include <range/v3/view.hpp>

#include <limits>
#include <deque>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;

SMTEncoder::SMTEncoder(
	smt::EncodingContext& _context,
	ModelCheckerSettings const& _settings,
	UniqueErrorReporter& _errorReporter,
	langutil::CharStreamProvider const& _charStreamProvider
):
	m_errorReporter(_errorReporter),
	m_context(_context),
	m_settings(_settings),
	m_charStreamProvider(_charStreamProvider)
{
}

void SMTEncoder::resetSourceAnalysis()
{
	m_freeFunctions.clear();
}

bool SMTEncoder::visit(ContractDefinition const& _contract)
{
	solAssert(m_currentContract, "");

	for (auto const& node: _contract.subNodes())
		if (
			!dynamic_pointer_cast<FunctionDefinition>(node) &&
			!dynamic_pointer_cast<VariableDeclaration>(node)
		)
			node->accept(*this);

	for (auto const& base: _contract.annotation().linearizedBaseContracts)
	{
		// Look for all the constructor invocations bottom up.
		if (auto const& constructor =  base->constructor())
			for (auto const& invocation: constructor->modifiers())
			{
				auto refDecl = invocation->name().annotation().referencedDeclaration;
				if (auto const& baseContract = dynamic_cast<ContractDefinition const*>(refDecl))
				{
					solAssert(!m_baseConstructorCalls.count(baseContract), "");
					m_baseConstructorCalls[baseContract] = invocation.get();
				}
			}
	}
	// Functions are visited first since they might be used
	// for state variable initialization which is part of
	// the constructor.
	// Constructors are visited as part of the constructor
	// hierarchy inlining.
	for (auto const* function: contractFunctionsWithoutVirtual(_contract) + allFreeFunctions())
		if (!function->isConstructor())
			function->accept(*this);

	// Constructors need to be handled by the engines separately.

	return false;
}

void SMTEncoder::endVisit(ContractDefinition const& _contract)
{
	m_context.resetAllVariables();

	m_baseConstructorCalls.clear();

	solAssert(m_currentContract == &_contract, "");
	m_currentContract = nullptr;

	if (m_callStack.empty())
		m_context.popSolver();
}

bool SMTEncoder::visit(ImportDirective const&)
{
	// do not visit because the identifier therein will confuse us.
	return false;
}

void SMTEncoder::endVisit(VariableDeclaration const& _varDecl)
{
	// State variables are handled by the constructor.
	if (_varDecl.isLocalVariable() &&_varDecl.value())
		assignment(_varDecl, *_varDecl.value());
}

bool SMTEncoder::visit(ModifierDefinition const&)
{
	return false;
}

bool SMTEncoder::visit(FunctionDefinition const& _function)
{
	solAssert(m_currentContract, "");

	m_modifierDepthStack.push_back(-1);

	initializeLocalVariables(_function);

	_function.parameterList().accept(*this);
	if (_function.returnParameterList())
		_function.returnParameterList()->accept(*this);

	visitFunctionOrModifier();

	return false;
}

void SMTEncoder::visitFunctionOrModifier()
{
	solAssert(!m_callStack.empty(), "");
	solAssert(!m_modifierDepthStack.empty(), "");

	++m_modifierDepthStack.back();
	FunctionDefinition const& function = dynamic_cast<FunctionDefinition const&>(*m_callStack.back().first);

	if (m_modifierDepthStack.back() == static_cast<int>(function.modifiers().size()))
	{
		if (function.isImplemented())
		{
			pushInlineFrame(function);
			function.body().accept(*this);
			popInlineFrame(function);
		}
	}
	else
	{
		solAssert(m_modifierDepthStack.back() < static_cast<int>(function.modifiers().size()), "");
		ASTPointer<ModifierInvocation> const& modifierInvocation =
			function.modifiers()[static_cast<size_t>(m_modifierDepthStack.back())];
		solAssert(modifierInvocation, "");
		auto refDecl = modifierInvocation->name().annotation().referencedDeclaration;
		if (dynamic_cast<ContractDefinition const*>(refDecl))
			visitFunctionOrModifier();
		else if (auto modifier = resolveModifierInvocation(*modifierInvocation, m_currentContract))
		{
			m_scopes.push_back(modifier);
			inlineModifierInvocation(modifierInvocation.get(), modifier);
			solAssert(m_scopes.back() == modifier, "");
			m_scopes.pop_back();
		}
		else
			solAssert(false, "");
	}

	--m_modifierDepthStack.back();
}

void SMTEncoder::inlineModifierInvocation(ModifierInvocation const* _invocation, CallableDeclaration const* _definition)
{
	solAssert(_invocation, "");
	_invocation->accept(*this);

	vector<smtutil::Expression> args;
	if (auto const* arguments = _invocation->arguments())
	{
		auto const& modifierParams = _definition->parameters();
		solAssert(modifierParams.size() == arguments->size(), "");
		for (unsigned i = 0; i < arguments->size(); ++i)
			args.push_back(expr(*arguments->at(i), modifierParams.at(i)->type()));
	}

	initializeFunctionCallParameters(*_definition, args);

	pushCallStack({_definition, _invocation});
	pushInlineFrame(*_definition);
	if (auto modifier = dynamic_cast<ModifierDefinition const*>(_definition))
	{
		if (modifier->isImplemented())
			modifier->body().accept(*this);
		popCallStack();
	}
	else if (auto function = dynamic_cast<FunctionDefinition const*>(_definition))
	{
		if (function->isImplemented())
			function->accept(*this);
		// Functions are popped from the callstack in endVisit(FunctionDefinition)
	}
	popInlineFrame(*_definition);
}

void SMTEncoder::inlineConstructorHierarchy(ContractDefinition const& _contract)
{
	auto const& hierarchy = m_currentContract->annotation().linearizedBaseContracts;
	auto it = find(begin(hierarchy), end(hierarchy), &_contract);
	solAssert(it != end(hierarchy), "");

	auto nextBase = it + 1;
	// Initialize the base contracts here as long as their constructors are implicit,
	// stop when the first explicit constructor is found.
	while (nextBase != end(hierarchy))
	{
		if (auto baseConstructor = (*nextBase)->constructor())
		{
			createLocalVariables(*baseConstructor);
			// If any subcontract explicitly called baseConstructor, use those arguments.
			if (m_baseConstructorCalls.count(*nextBase))
				inlineModifierInvocation(m_baseConstructorCalls.at(*nextBase), baseConstructor);
			else if (baseConstructor->isImplemented())
			{
				// The first constructor found is handled like a function
				// and its pushed into the callstack there.
				// This if avoids duplication in the callstack.
				if (!m_callStack.empty())
					pushCallStack({baseConstructor, nullptr});
				baseConstructor->accept(*this);
				// popped by endVisit(FunctionDefinition)
			}
			break;
		}
		else
		{
			initializeStateVariables(**nextBase);
			++nextBase;
		}
	}

	initializeStateVariables(_contract);
}

bool SMTEncoder::visit(PlaceholderStatement const&)
{
	solAssert(!m_callStack.empty(), "");
	auto lastCall = popCallStack();
	visitFunctionOrModifier();
	pushCallStack(lastCall);
	return true;
}

void SMTEncoder::endVisit(FunctionDefinition const&)
{
	solAssert(m_currentContract, "");

	popCallStack();
	solAssert(m_modifierDepthStack.back() == -1, "");
	m_modifierDepthStack.pop_back();
	if (m_callStack.empty())
		m_context.popSolver();
}

bool SMTEncoder::visit(Block const& _block)
{
	if (_block.unchecked())
	{
		solAssert(m_checked, "");
		m_checked = false;
	}
	return true;
}

void SMTEncoder::endVisit(Block const& _block)
{
	if (_block.unchecked())
	{
		solAssert(!m_checked, "");
		m_checked = true;
	}
}

bool SMTEncoder::visit(InlineAssembly const& _inlineAsm)
{
	/// This is very similar to `yul::Assignments`, except I need to collect `Identifier`s and not just names as `YulString`s.
	struct AssignedExternalsCollector: public yul::ASTWalker
	{
		AssignedExternalsCollector(InlineAssembly const& _inlineAsm): externalReferences(_inlineAsm.annotation().externalReferences)
		{
			this->operator()(_inlineAsm.operations());
		}

		map<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo> const& externalReferences;
		set<VariableDeclaration const*> assignedVars;

		using yul::ASTWalker::operator();
		void operator()(yul::Assignment const& _assignment)
		{
			auto const& vars = _assignment.variableNames;
			for (auto const& identifier: vars)
				if (auto externalInfo = util::valueOrNullptr(externalReferences, &identifier))
					if (auto varDecl = dynamic_cast<VariableDeclaration const*>(externalInfo->declaration))
						assignedVars.insert(varDecl);
		}
	};

	yul::SideEffectsCollector sideEffectsCollector(_inlineAsm.dialect(), _inlineAsm.operations());
	if (sideEffectsCollector.invalidatesMemory())
		resetMemoryVariables();
	if (sideEffectsCollector.invalidatesStorage())
		resetStorageVariables();

	auto assignedVars = AssignedExternalsCollector(_inlineAsm).assignedVars;
	for (auto const* var: assignedVars)
	{
		solAssert(var, "");
		solAssert(var->isLocalVariable(), "Non-local variable assigned in inlined assembly");
		m_context.resetVariable(*var);
	}

	m_errorReporter.warning(
		7737_error,
		_inlineAsm.location(),
		"Inline assembly may cause SMTChecker to produce spurious warnings (false positives)."
	);
	return false;
}

void SMTEncoder::pushInlineFrame(CallableDeclaration const&)
{
	pushPathCondition(currentPathConditions());
}

void SMTEncoder::popInlineFrame(CallableDeclaration const&)
{
	popPathCondition();
}

void SMTEncoder::endVisit(VariableDeclarationStatement const& _varDecl)
{
	if (auto init = _varDecl.initialValue())
		expressionToTupleAssignment(_varDecl.declarations(), *init);
}

bool SMTEncoder::visit(Assignment const& _assignment)
{
	// RHS must be visited before LHS; as opposed to what Assignment::accept does
	_assignment.rightHandSide().accept(*this);
	_assignment.leftHandSide().accept(*this);
	return false;
}

void SMTEncoder::endVisit(Assignment const& _assignment)
{
	createExpr(_assignment);

	Token op = _assignment.assignmentOperator();
	solAssert(TokenTraits::isAssignmentOp(op), "");

	if (!isSupportedType(*_assignment.annotation().type))
	{
		// Give it a new index anyway to keep the SSA scheme sound.

		Expression const* base = &_assignment.leftHandSide();
		if (auto const* indexAccess = dynamic_cast<IndexAccess const*>(base))
			base = leftmostBase(*indexAccess);

		if (auto varDecl = identifierToVariable(*base))
			m_context.newValue(*varDecl);
	}
	else
	{
		if (dynamic_cast<TupleType const*>(_assignment.rightHandSide().annotation().type))
			tupleAssignment(_assignment.leftHandSide(), _assignment.rightHandSide());
		else
		{
			auto const& type = _assignment.annotation().type;
			auto rightHandSide = op == Token::Assign ?
				expr(_assignment.rightHandSide(), type) :
				compoundAssignment(_assignment);
			defineExpr(_assignment, rightHandSide);
			assignment(
				_assignment.leftHandSide(),
				expr(_assignment, type),
				type
			);
		}
	}
}

void SMTEncoder::endVisit(TupleExpression const& _tuple)
{
	if (_tuple.annotation().type->category() == Type::Category::Function)
		return;

	if (_tuple.annotation().type->category() == Type::Category::TypeType)
		return;

	createExpr(_tuple);

	if (_tuple.isInlineArray())
	{
		// Add constraints for the length and values as it is known.
		auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(_tuple));
		solAssert(symbArray, "");

		addArrayLiteralAssertions(*symbArray, applyMap(_tuple.components(), [&](auto const& c) { return expr(*c); }));
	}
	else
	{
		auto values = applyMap(_tuple.components(), [this](auto const& component) -> optional<smtutil::Expression> {
			if (component)
			{
				if (!m_context.knownExpression(*component))
						createExpr(*component);
				return expr(*component);
			}
			return {};
		});
		defineExpr(_tuple, values);
	}
}

void SMTEncoder::endVisit(UnaryOperation const& _op)
{
	/// We need to shortcut here due to potentially unknown
	/// rational number sizes.
	if (_op.annotation().type->category() == Type::Category::RationalNumber)
		return;

	if (TokenTraits::isBitOp(_op.getOperator()))
		return bitwiseNotOperation(_op);

	createExpr(_op);

	auto const* subExpr = innermostTuple(_op.subExpression());
	auto type = _op.annotation().type;
	switch (_op.getOperator())
	{
	case Token::Not: // !
	{
		solAssert(smt::isBool(*type), "");
		defineExpr(_op, !expr(*subExpr));
		break;
	}
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
	{
		solAssert(smt::isInteger(*type) || smt::isFixedPoint(*type), "");
		solAssert(subExpr->annotation().willBeWrittenTo, "");
		auto innerValue = expr(*subExpr);
		auto newValue = arithmeticOperation(
			_op.getOperator() == Token::Inc ? Token::Add : Token::Sub,
			innerValue,
			smtutil::Expression(size_t(1)),
			_op.annotation().type,
			_op
		).first;
		defineExpr(_op, _op.isPrefixOperation() ? newValue : innerValue);
		assignment(*subExpr, newValue);
		break;
	}
	case Token::Sub: // -
	{
		defineExpr(_op, 0 - expr(*subExpr));
		break;
	}
	case Token::Delete:
	{
		if (auto decl = identifierToVariable(*subExpr))
		{
			m_context.newValue(*decl);
			m_context.setZeroValue(*decl);
		}
		else
		{
			solAssert(m_context.knownExpression(*subExpr), "");
			auto const& symbVar = m_context.expression(*subExpr);
			symbVar->increaseIndex();
			m_context.setZeroValue(*symbVar);
			if (
				dynamic_cast<IndexAccess const*>(subExpr) ||
				dynamic_cast<MemberAccess const*>(subExpr)
			)
				indexOrMemberAssignment(*subExpr, symbVar->currentValue());
			// Empty push added a zero value anyway, so no need to delete extra.
			else if (!isEmptyPush(*subExpr))
				solAssert(false, "");
		}
		break;
	}
	default:
		solAssert(false, "");
	}
}

bool SMTEncoder::visit(UnaryOperation const& _op)
{
	return !shortcutRationalNumber(_op);
}

bool SMTEncoder::visit(BinaryOperation const& _op)
{
	if (shortcutRationalNumber(_op))
		return false;
	if (TokenTraits::isBooleanOp(_op.getOperator()))
	{
		booleanOperation(_op);
		return false;
	}
	return true;
}

void SMTEncoder::endVisit(BinaryOperation const& _op)
{
	/// If _op is const evaluated the RationalNumber shortcut was taken.
	if (isConstant(_op))
		return;
	if (TokenTraits::isBooleanOp(_op.getOperator()))
		return;

	createExpr(_op);

	if (TokenTraits::isArithmeticOp(_op.getOperator()))
		arithmeticOperation(_op);
	else if (TokenTraits::isCompareOp(_op.getOperator()))
		compareOperation(_op);
	else if (TokenTraits::isBitOp(_op.getOperator()) || TokenTraits::isShiftOp(_op.getOperator()))
		bitwiseOperation(_op);
	else
		solAssert(false, "");
}

bool SMTEncoder::visit(Conditional const& _op)
{
	_op.condition().accept(*this);

	auto indicesEndTrue = visitBranch(&_op.trueExpression(), expr(_op.condition())).first;

	auto indicesEndFalse = visitBranch(&_op.falseExpression(), !expr(_op.condition())).first;

	mergeVariables(expr(_op.condition()), indicesEndTrue, indicesEndFalse);

	defineExpr(_op, smtutil::Expression::ite(
		expr(_op.condition()),
		expr(_op.trueExpression(), _op.annotation().type),
		expr(_op.falseExpression(), _op.annotation().type)
	));

	return false;
}

bool SMTEncoder::visit(FunctionCall const& _funCall)
{
	auto functionCallKind = *_funCall.annotation().kind;
	if (functionCallKind != FunctionCallKind::FunctionCall)
		return true;

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	// We do not want to visit the TypeTypes in the second argument of `abi.decode`.
	// Those types are checked/used in SymbolicState::buildABIFunctions.
	if (funType.kind() == FunctionType::Kind::ABIDecode)
	{
		if (auto arg = _funCall.arguments().front())
			arg->accept(*this);
		return false;
	}
	else if (funType.kind() == FunctionType::Kind::ABIEncodeCall)
	{
		auto fun = _funCall.arguments().front();
		createExpr(*fun);
		auto const* functionType = dynamic_cast<FunctionType const*>(fun->annotation().type);
		if (functionType->hasDeclaration())
			defineExpr(*fun, functionType->externalIdentifier());
		return true;
	}

	// We do not really need to visit the expression in a wrap/unwrap no-op call,
	// so we just ignore the function call expression to avoid "unsupported" warnings.
	else if (
		funType.kind() == FunctionType::Kind::Wrap ||
		funType.kind() == FunctionType::Kind::Unwrap
	)
	{
		if (auto arg = _funCall.arguments().front())
			arg->accept(*this);
		return false;
	}
	return true;
}

void SMTEncoder::endVisit(FunctionCall const& _funCall)
{
	auto functionCallKind = *_funCall.annotation().kind;

	createExpr(_funCall);
	if (functionCallKind == FunctionCallKind::StructConstructorCall)
	{
		visitStructConstructorCall(_funCall);
		return;
	}

	if (functionCallKind == FunctionCallKind::TypeConversion)
	{
		visitTypeConversion(_funCall);
		return;
	}

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);

	std::vector<ASTPointer<Expression const>> const args = _funCall.arguments();
	switch (funType.kind())
	{
	case FunctionType::Kind::Assert:
		visitAssert(_funCall);
		break;
	case FunctionType::Kind::Require:
		visitRequire(_funCall);
		break;
	case FunctionType::Kind::Revert:
		// Revert is a special case of require and equals to `require(false)`
		addPathImpliedExpression(smtutil::Expression(false));
		break;
	case FunctionType::Kind::GasLeft:
		visitGasLeft(_funCall);
		break;
	case FunctionType::Kind::External:
		if (isPublicGetter(_funCall.expression()))
			visitPublicGetter(_funCall);
		break;
	case FunctionType::Kind::ABIDecode:
	case FunctionType::Kind::ABIEncode:
	case FunctionType::Kind::ABIEncodePacked:
	case FunctionType::Kind::ABIEncodeWithSelector:
	case FunctionType::Kind::ABIEncodeCall:
	case FunctionType::Kind::ABIEncodeWithSignature:
		visitABIFunction(_funCall);
		break;
	case FunctionType::Kind::Internal:
	case FunctionType::Kind::BareStaticCall:
	case FunctionType::Kind::BareCall:
		break;
	case FunctionType::Kind::KECCAK256:
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::SHA256:
	case FunctionType::Kind::RIPEMD160:
		visitCryptoFunction(_funCall);
		break;
	case FunctionType::Kind::BlockHash:
		defineExpr(_funCall, state().blockhash(expr(*_funCall.arguments().at(0))));
		break;
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
		visitAddMulMod(_funCall);
		break;
	case FunctionType::Kind::Unwrap:
	case FunctionType::Kind::Wrap:
		visitWrapUnwrap(_funCall);
		break;
	case FunctionType::Kind::Send:
	case FunctionType::Kind::Transfer:
	{
		auto const& memberAccess = dynamic_cast<MemberAccess const&>(_funCall.expression());
		auto const& address = memberAccess.expression();
		auto const& value = args.front();
		solAssert(value, "");

		smtutil::Expression thisBalance = state().balance();
		setSymbolicUnknownValue(thisBalance, TypeProvider::uint256(), m_context);

		state().transfer(state().thisAddress(), expr(address), expr(*value));
		break;
	}
	case FunctionType::Kind::ArrayPush:
		arrayPush(_funCall);
		break;
	case FunctionType::Kind::ArrayPop:
		arrayPop(_funCall);
		break;
	case FunctionType::Kind::Event:
	case FunctionType::Kind::Error:
		// This can be safely ignored.
		break;
	case FunctionType::Kind::ObjectCreation:
		visitObjectCreation(_funCall);
		return;
	case FunctionType::Kind::DelegateCall:
	case FunctionType::Kind::BareCallCode:
	case FunctionType::Kind::BareDelegateCall:
	case FunctionType::Kind::Creation:
	default:
		m_errorReporter.warning(
			4588_error,
			_funCall.location(),
			"Assertion checker does not yet implement this type of function call."
		);
	}
}

bool SMTEncoder::visit(ModifierInvocation const& _node)
{
	if (auto const* args = _node.arguments())
		for (auto const& arg: *args)
			if (arg)
				arg->accept(*this);
	return false;
}

void SMTEncoder::initContract(ContractDefinition const& _contract)
{
	solAssert(m_currentContract == nullptr, "");
	m_currentContract = &_contract;

	m_context.reset();
	m_context.pushSolver();
	createStateVariables(_contract);
	clearIndices(m_currentContract, nullptr);
	m_variableUsage.setCurrentContract(_contract);
	m_checked = true;
}

void SMTEncoder::initFunction(FunctionDefinition const& _function)
{
	solAssert(m_callStack.empty(), "");
	solAssert(m_currentContract, "");
	m_context.pushSolver();
	m_pathConditions.clear();
	pushCallStack({&_function, nullptr});
	m_uninterpretedTerms.clear();
	createStateVariables(*m_currentContract);
	createLocalVariables(_function);
	m_arrayAssignmentHappened = false;
	clearIndices(m_currentContract, &_function);
	m_checked = true;
}

void SMTEncoder::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
}

void SMTEncoder::visitRequire(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() >= 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
	addPathImpliedExpression(expr(*args.front()));
}

void SMTEncoder::visitABIFunction(FunctionCall const& _funCall)
{
	auto symbFunction = state().abiFunction(&_funCall);
	auto const& [name, inTypes, outTypes] = state().abiFunctionTypes(&_funCall);

	auto const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	auto const& args = _funCall.sortedArguments();
	auto argsActualLength = kind == FunctionType::Kind::ABIDecode ? 1 : args.size();

	solAssert(inTypes.size() == argsActualLength, "");
	if (argsActualLength == 0)
	{
		defineExpr(_funCall, smt::zeroValue(TypeProvider::bytesMemory()));
		return;
	}
	vector<smtutil::Expression> symbArgs;
	for (unsigned i = 0; i < argsActualLength; ++i)
		if (args.at(i))
			symbArgs.emplace_back(expr(*args.at(i), inTypes.at(i)));

	optional<smtutil::Expression> arg;
	if (inTypes.size() == 1)
		arg = expr(*args.at(0), inTypes.at(0));
	else
	{
		auto inputSort = dynamic_cast<smtutil::ArraySort&>(*symbFunction.sort).domain;
		arg = smtutil::Expression::tuple_constructor(
			smtutil::Expression(make_shared<smtutil::SortSort>(inputSort), ""),
			symbArgs
		);
	}

	auto out = smtutil::Expression::select(symbFunction, *arg);
	if (outTypes.size() == 1)
		defineExpr(_funCall, out);
	else
	{
		auto symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(_funCall));
		solAssert(symbTuple, "");
		solAssert(symbTuple->components().size() == outTypes.size(), "");
		solAssert(out.sort->kind == smtutil::Kind::Tuple, "");

		symbTuple->increaseIndex();
		for (unsigned i = 0; i < symbTuple->components().size(); ++i)
			m_context.addAssertion(symbTuple->component(i) == smtutil::Expression::tuple_get(out, i));
	}
}

void SMTEncoder::visitCryptoFunction(FunctionCall const& _funCall)
{
	auto const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	auto arg0 = expr(*_funCall.arguments().at(0));
	optional<smtutil::Expression> result;
	if (kind == FunctionType::Kind::KECCAK256)
		result = smtutil::Expression::select(state().cryptoFunction("keccak256"), arg0);
	else if (kind == FunctionType::Kind::SHA256)
		result = smtutil::Expression::select(state().cryptoFunction("sha256"), arg0);
	else if (kind == FunctionType::Kind::RIPEMD160)
		result = smtutil::Expression::select(state().cryptoFunction("ripemd160"), arg0);
	else if (kind == FunctionType::Kind::ECRecover)
	{
		auto e = state().cryptoFunction("ecrecover");
		auto arg0 = expr(*_funCall.arguments().at(0));
		auto arg1 = expr(*_funCall.arguments().at(1));
		auto arg2 = expr(*_funCall.arguments().at(2));
		auto arg3 = expr(*_funCall.arguments().at(3));
		auto inputSort = dynamic_cast<smtutil::ArraySort&>(*e.sort).domain;
		auto ecrecoverInput = smtutil::Expression::tuple_constructor(
			smtutil::Expression(make_shared<smtutil::SortSort>(inputSort), ""),
			{arg0, arg1, arg2, arg3}
		);
		result = smtutil::Expression::select(e, ecrecoverInput);
	}
	else
		solAssert(false, "");

	defineExpr(_funCall, *result);
}

void SMTEncoder::visitGasLeft(FunctionCall const& _funCall)
{
	string gasLeft = "gasleft";
	// We increase the variable index since gasleft changes
	// inside a tx.
	defineGlobalVariable(gasLeft, _funCall, true);
	auto const& symbolicVar = m_context.globalSymbol(gasLeft);
	unsigned index = symbolicVar->index();
	// We set the current value to unknown anyway to add type constraints.
	m_context.setUnknownValue(*symbolicVar);
	if (index > 0)
		m_context.addAssertion(symbolicVar->currentValue() <= symbolicVar->valueAtIndex(index - 1));
}

void SMTEncoder::visitAddMulMod(FunctionCall const& _funCall)
{
	auto const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	solAssert(kind == FunctionType::Kind::AddMod || kind == FunctionType::Kind::MulMod, "");
	auto const& args = _funCall.arguments();
	solAssert(args.at(0) && args.at(1) && args.at(2), "");
	auto x = expr(*args.at(0));
	auto y = expr(*args.at(1));
	auto k = expr(*args.at(2));
	auto const& intType = dynamic_cast<IntegerType const&>(*_funCall.annotation().type);

	if (kind == FunctionType::Kind::AddMod)
		defineExpr(_funCall, divModWithSlacks(x + y, k, intType).second);
	else
		defineExpr(_funCall, divModWithSlacks(x * y, k, intType).second);
}

void SMTEncoder::visitWrapUnwrap(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	defineExpr(_funCall, expr(*args.front()));
}

void SMTEncoder::visitObjectCreation(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() >= 1, "");
	auto argType = args.front()->annotation().type->category();
	solAssert(argType == Type::Category::Integer || argType == Type::Category::RationalNumber, "");

	smtutil::Expression arraySize = expr(*args.front());
	setSymbolicUnknownValue(arraySize, TypeProvider::uint256(), m_context);

	auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(_funCall));
	solAssert(symbArray, "");
	smt::setSymbolicZeroValue(*symbArray, m_context);
	auto zeroElements = symbArray->elements();
	symbArray->increaseIndex();
	m_context.addAssertion(symbArray->length() == arraySize);
	m_context.addAssertion(symbArray->elements() == zeroElements);
}

void SMTEncoder::endVisit(Identifier const& _identifier)
{
	if (auto decl = identifierToVariable(_identifier))
	{
		if (decl->isConstant())
			defineExpr(_identifier, constantExpr(_identifier, *decl));
		else
			defineExpr(_identifier, currentValue(*decl));
	}
	else if (_identifier.annotation().type->category() == Type::Category::Function)
		visitFunctionIdentifier(_identifier);
	else if (_identifier.name() == "now")
		defineGlobalVariable(_identifier.name(), _identifier);
	else if (_identifier.name() == "this")
	{
		defineExpr(_identifier, state().thisAddress());
		m_uninterpretedTerms.insert(&_identifier);
	}
	// Ignore type identifiers
	else if (dynamic_cast<TypeType const*>(_identifier.annotation().type))
		return;
	// Ignore module identifiers
	else if (dynamic_cast<ModuleType const*>(_identifier.annotation().type))
		return;
	// Ignore user defined value type identifiers
	else if (dynamic_cast<UserDefinedValueType const*>(_identifier.annotation().type))
		return;
	// Ignore the builtin abi, it is handled in FunctionCall.
	// TODO: ignore MagicType in general (abi, block, msg, tx, type)
	else if (auto magicType = dynamic_cast<MagicType const*>(_identifier.annotation().type); magicType && magicType->kind() == MagicType::Kind::ABI)
	{
		solAssert(_identifier.name() == "abi", "");
		return;
	}
	else
		createExpr(_identifier);
}

void SMTEncoder::endVisit(ElementaryTypeNameExpression const& _typeName)
{
	auto const& typeType = dynamic_cast<TypeType const&>(*_typeName.annotation().type);
	auto result = smt::newSymbolicVariable(
		*TypeProvider::uint256(),
		typeType.actualType()->toString(false),
		m_context
	);
	solAssert(!result.first && result.second, "");
	m_context.createExpression(_typeName, result.second);
}

namespace // helpers for SMTEncoder::visitPublicGetter
{

bool isReturnedFromStructGetter(Type const* _type)
{
	// So far it seems that only Mappings and ordinary Arrays are not returned.
	auto category = _type->category();
	if (category == Type::Category::Mapping)
		return false;
	if (category == Type::Category::Array)
		return dynamic_cast<ArrayType const&>(*_type).isByteArrayOrString();
	// default
	return true;
}

vector<string> structGetterReturnedMembers(StructType const& _structType)
{
	vector<string> returnedMembers;
	for (auto const& member: _structType.nativeMembers(nullptr))
		if (isReturnedFromStructGetter(member.type))
			returnedMembers.push_back(member.name);
	return returnedMembers;
}

}

void SMTEncoder::visitPublicGetter(FunctionCall const& _funCall)
{
	MemberAccess const& access = dynamic_cast<MemberAccess const&>(_funCall.expression());
	auto var = dynamic_cast<VariableDeclaration const*>(access.annotation().referencedDeclaration);
	solAssert(var, "");
	solAssert(m_context.knownExpression(_funCall), "");
	auto paramExpectedTypes = replaceUserTypes(FunctionType(*var).parameterTypes());
	auto actualArguments = _funCall.arguments();
	solAssert(actualArguments.size() == paramExpectedTypes.size(), "");
	deque<smtutil::Expression> symbArguments;
	for (unsigned i = 0; i < paramExpectedTypes.size(); ++i)
		symbArguments.push_back(expr(*actualArguments[i], paramExpectedTypes[i]));

	// See FunctionType::FunctionType(VariableDeclaration const& _varDecl)
	// to understand the return types of public getters.
	Type const* type = var->type();
	smtutil::Expression currentExpr = currentValue(*var);
	while (true)
	{
		if (
			type->isValueType() ||
			(type->category() == Type::Category::Array && dynamic_cast<ArrayType const&>(*type).isByteArrayOrString())
		)
		{
			solAssert(symbArguments.empty(), "");
			defineExpr(_funCall, currentExpr);
			return;
		}
		switch (type->category())
		{
			case Type::Category::Array:
			case Type::Category::Mapping:
			{
				solAssert(!symbArguments.empty(), "");
				// For nested arrays/mappings, each argument in the call is an index to the next layer.
				// We mirror this with `select` after unpacking the SMT-LIB array expression.
				currentExpr = smtutil::Expression::select(smtutil::Expression::tuple_get(currentExpr, 0), symbArguments.front());
				symbArguments.pop_front();
				if (auto arrayType = dynamic_cast<ArrayType const*>(type))
					type = arrayType->baseType();
				else if (auto mappingType = dynamic_cast<MappingType const*>(type))
					type = mappingType->valueType();
				else
					solAssert(false, "");
				break;
			}
			case Type::Category::Struct:
			{
				solAssert(symbArguments.empty(), "");
				smt::SymbolicStructVariable structVar(dynamic_cast<StructType const*>(type), "struct_temp_" + to_string(_funCall.id()), m_context);
				m_context.addAssertion(structVar.currentValue() == currentExpr);
				auto returnedMembers = structGetterReturnedMembers(dynamic_cast<StructType const&>(*structVar.type()));
				solAssert(!returnedMembers.empty(), "");
				auto returnedValues = applyMap(returnedMembers, [&](string const& memberName) -> optional<smtutil::Expression> { return structVar.member(memberName); });
				defineExpr(_funCall, returnedValues);
				return;
			}
			default:
			{
				// Unsupported cases, do nothing and the getter will be abstracted.
				return;
			}
		}
	}
}

bool SMTEncoder::shouldAnalyze(SourceUnit const& _source) const
{
	return m_settings.contracts.isDefault() ||
		m_settings.contracts.has(*_source.annotation().path);
}

bool SMTEncoder::shouldAnalyze(ContractDefinition const& _contract) const
{
	if (!_contract.canBeDeployed())
		return false;

	return m_settings.contracts.isDefault() ||
		m_settings.contracts.has(_contract.sourceUnitName(), _contract.name());
}

void SMTEncoder::visitTypeConversion(FunctionCall const& _funCall)
{
	solAssert(*_funCall.annotation().kind == FunctionCallKind::TypeConversion, "");
	solAssert(_funCall.arguments().size() == 1, "");

	auto argument = _funCall.arguments().front();
	auto const argType = argument->annotation().type;
	auto const funCallType = _funCall.annotation().type;

	auto symbArg = expr(*argument, funCallType);

	if (smt::isStringLiteral(*argType) && smt::isFixedBytes(*funCallType))
	{
		defineExpr(_funCall, symbArg);
		return;
	}

	ArrayType const* arrayType = dynamic_cast<ArrayType const*>(argType);
	if (auto sliceType = dynamic_cast<ArraySliceType const*>(argType))
		arrayType = &sliceType->arrayType();

	if (arrayType && arrayType->isByteArrayOrString() && smt::isFixedBytes(*funCallType))
	{
		auto array = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(*argument));
		bytesToFixedBytesAssertions(*array, _funCall);
		return;
	}

	// TODO Simplify this whole thing for 0.8.0 where weird casts are disallowed.

	unsigned argSize = argType->storageBytes();
	unsigned castSize = funCallType->storageBytes();
	bool castIsSigned = smt::isNumber(*funCallType) && smt::isSigned(funCallType);
	bool argIsSigned = smt::isNumber(*argType) && smt::isSigned(argType);
	optional<smtutil::Expression> symbMin;
	optional<smtutil::Expression> symbMax;
	if (smt::isNumber(*funCallType))
	{
		symbMin = smt::minValue(funCallType);
		symbMax = smt::maxValue(funCallType);
	}
	if (argSize == castSize)
	{
		// If sizes are the same, it's possible that the signs are different.
		if (smt::isNumber(*funCallType) && smt::isNumber(*argType))
		{
			// castIsSigned && !argIsSigned => might overflow if arg > castType.max
			// !castIsSigned && argIsSigned => might underflow if arg < castType.min
			// !castIsSigned && !argIsSigned => ok
			// castIsSigned && argIsSigned => ok

			if (castIsSigned && !argIsSigned)
			{
				auto wrap = smtutil::Expression::ite(
					symbArg > *symbMax,
					symbArg - (*symbMax - *symbMin + 1),
					symbArg
				);
				defineExpr(_funCall, wrap);
			}
			else if (!castIsSigned && argIsSigned)
			{
				auto wrap = smtutil::Expression::ite(
					symbArg < *symbMin,
					symbArg + (*symbMax + 1),
					symbArg
				);
				defineExpr(_funCall, wrap);
			}
			else
				defineExpr(_funCall, symbArg);
		}
		else
			defineExpr(_funCall, symbArg);
	}
	else if (castSize > argSize)
	{
		solAssert(smt::isNumber(*funCallType), "");
		// RationalNumbers have size 32.
		solAssert(argType->category() != Type::Category::RationalNumber, "");

		// castIsSigned && !argIsSigned => ok
		// castIsSigned && argIsSigned => ok
		// !castIsSigned && !argIsSigned => ok except for FixedBytesType, need to adjust padding
		// !castIsSigned && argIsSigned => might underflow if arg < castType.min

		if (!castIsSigned && argIsSigned)
		{
			auto wrap = smtutil::Expression::ite(
				symbArg < *symbMin,
				symbArg + (*symbMax + 1),
				symbArg
			);
			defineExpr(_funCall, wrap);
		}
		else if (!castIsSigned && !argIsSigned)
		{
			if (auto const* fixedCast = dynamic_cast<FixedBytesType const*>(funCallType))
			{
				auto const* fixedArg = dynamic_cast<FixedBytesType const*>(argType);
				solAssert(fixedArg, "");
				auto diff = fixedCast->numBytes() - fixedArg->numBytes();
				solAssert(diff > 0, "");
				auto bvSize = fixedCast->numBytes() * 8;
				defineExpr(
					_funCall,
					smtutil::Expression::bv2int(smtutil::Expression::int2bv(symbArg, bvSize) << smtutil::Expression::int2bv(diff * 8, bvSize))
				);
			}
			else
				defineExpr(_funCall, symbArg);
		}
		else
			defineExpr(_funCall, symbArg);
	}
	else // castSize < argSize
	{
		solAssert(smt::isNumber(*funCallType), "");

		RationalNumberType const* rationalType = isConstant(*argument);
		if (rationalType)
		{
			// The TypeChecker guarantees that a constant fits in the cast size.
			defineExpr(_funCall, symbArg);
			return;
		}

		auto const* fixedCast = dynamic_cast<FixedBytesType const*>(funCallType);
		auto const* fixedArg = dynamic_cast<FixedBytesType const*>(argType);
		if (fixedCast && fixedArg)
		{
			createExpr(_funCall);
			auto diff = argSize - castSize;
			solAssert(fixedArg->numBytes() - fixedCast->numBytes() == diff, "");

			auto argValueBV = smtutil::Expression::int2bv(symbArg, argSize * 8);
			auto shr = smtutil::Expression::int2bv(diff * 8, argSize * 8);
			solAssert(!castIsSigned, "");
			defineExpr(_funCall, smtutil::Expression::bv2int(argValueBV >> shr));
		}
		else
		{
			auto argValueBV = smtutil::Expression::int2bv(symbArg, castSize * 8);
			defineExpr(_funCall, smtutil::Expression::bv2int(argValueBV, castIsSigned));
		}
	}
}

void SMTEncoder::visitFunctionIdentifier(Identifier const& _identifier)
{
	auto const& fType = dynamic_cast<FunctionType const&>(*_identifier.annotation().type);
	if (replaceUserTypes(fType.returnParameterTypes()).size() == 1)
	{
		defineGlobalVariable(fType.identifier(), _identifier);
		m_context.createExpression(_identifier, m_context.globalSymbol(fType.identifier()));
	}
}

void SMTEncoder::visitStructConstructorCall(FunctionCall const& _funCall)
{
	solAssert(*_funCall.annotation().kind == FunctionCallKind::StructConstructorCall, "");
	if (smt::isNonRecursiveStruct(*_funCall.annotation().type))
	{
		auto& structSymbolicVar = dynamic_cast<smt::SymbolicStructVariable&>(*m_context.expression(_funCall));
		auto structType = dynamic_cast<StructType const*>(structSymbolicVar.type());
		solAssert(structType, "");
		auto const& structMembers = structType->structDefinition().members();
		solAssert(structMembers.size() == _funCall.sortedArguments().size(), "");
		auto args = _funCall.sortedArguments();
		structSymbolicVar.assignAllMembers(applyMap(
			ranges::views::zip(args, structMembers),
			[this] (auto const& argMemberPair) { return expr(*argMemberPair.first, argMemberPair.second->type()); }
		));
	}

}

void SMTEncoder::endVisit(Literal const& _literal)
{
	solAssert(_literal.annotation().type, "Expected type for AST node");
	Type const& type = *_literal.annotation().type;
	if (smt::isNumber(type))
		defineExpr(_literal, smtutil::Expression(type.literalValue(&_literal)));
	else if (smt::isBool(type))
		defineExpr(_literal, smtutil::Expression(_literal.token() == Token::TrueLiteral ? true : false));
	else if (smt::isStringLiteral(type))
	{
		createExpr(_literal);

		// Add constraints for the length and values as it is known.
		auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(_literal));
		solAssert(symbArray, "");

		addArrayLiteralAssertions(
			*symbArray,
			applyMap(_literal.value(), [](unsigned char c) { return smtutil::Expression{size_t(c)}; })
		);
	}
	else
		solAssert(false, "");
}

void SMTEncoder::addArrayLiteralAssertions(
	smt::SymbolicArrayVariable& _symArray,
	vector<smtutil::Expression> const& _elementValues
)
{
	m_context.addAssertion(_symArray.length() == _elementValues.size());
	for (size_t i = 0; i < _elementValues.size(); i++)
		m_context.addAssertion(smtutil::Expression::select(_symArray.elements(), i) == _elementValues[i]);
}

void SMTEncoder::bytesToFixedBytesAssertions(
	smt::SymbolicArrayVariable& _symArray,
	Expression const& _fixedBytes
)
{
	auto const& fixed = dynamic_cast<FixedBytesType const&>(*_fixedBytes.annotation().type);
	auto intType = TypeProvider::uint256();
	string suffix = to_string(_fixedBytes.id()) + "_" + to_string(m_context.newUniqueId());
	smt::SymbolicIntVariable k(intType, intType, "k_" + suffix, m_context);
	m_context.addAssertion(k.currentValue() == 0);
	size_t n = fixed.numBytes();
	for (size_t i = 0; i < n; i++)
	{
		auto kPrev = k.currentValue();
		m_context.addAssertion((smtutil::Expression::select(_symArray.elements(), i) * (u256(1) << ((n - i - 1) * 8))) + kPrev == k.increaseIndex());
	}
	m_context.addAssertion(expr(_fixedBytes) == k.currentValue());
}

void SMTEncoder::endVisit(Return const& _return)
{
	if (_return.expression() && m_context.knownExpression(*_return.expression()))
	{
		auto returnParams = m_callStack.back().first->returnParameters();
		if (returnParams.size() > 1)
		{
			auto const& symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(*_return.expression()));
			solAssert(symbTuple, "");
			solAssert(symbTuple->components().size() == returnParams.size(), "");

			auto const* tupleType = dynamic_cast<TupleType const*>(_return.expression()->annotation().type);
			solAssert(tupleType, "");
			auto const& types = tupleType->components();
			solAssert(types.size() == returnParams.size(), "");

			for (unsigned i = 0; i < returnParams.size(); ++i)
				assignment(*returnParams.at(i), symbTuple->component(i, types.at(i), returnParams.at(i)->type()));
		}
		else if (returnParams.size() == 1)
			assignment(*returnParams.front(), expr(*_return.expression(), returnParams.front()->type()));
	}
}

bool SMTEncoder::visit(MemberAccess const& _memberAccess)
{
	auto const& accessType = _memberAccess.annotation().type;
	if (accessType->category() == Type::Category::Function)
		return true;

	createExpr(_memberAccess);

	Expression const* memberExpr = innermostTuple(_memberAccess.expression());

	auto const& exprType = memberExpr->annotation().type;
	solAssert(exprType, "");

	if (exprType->category() == Type::Category::Magic)
	{
		if (auto const* identifier = dynamic_cast<Identifier const*>(memberExpr))
		{
			auto const& name = identifier->name();
			solAssert(name == "block" || name == "msg" || name == "tx", "");
			defineExpr(_memberAccess, state().txMember(name + "." + _memberAccess.memberName()));
		}
		else if (auto magicType = dynamic_cast<MagicType const*>(exprType))
		{
			if (magicType->kind() == MagicType::Kind::Block)
				defineExpr(_memberAccess, state().txMember("block." + _memberAccess.memberName()));
			else if (magicType->kind() == MagicType::Kind::Message)
				defineExpr(_memberAccess, state().txMember("msg." + _memberAccess.memberName()));
			else if (magicType->kind() == MagicType::Kind::Transaction)
				defineExpr(_memberAccess, state().txMember("tx." + _memberAccess.memberName()));
			else if (magicType->kind() == MagicType::Kind::MetaType)
			{
				auto const& memberName = _memberAccess.memberName();
				if (memberName == "min" || memberName == "max")
				{
					if (IntegerType const* integerType = dynamic_cast<IntegerType const*>(magicType->typeArgument()))
						defineExpr(_memberAccess, memberName == "min" ? integerType->minValue() : integerType->maxValue());
					else if (EnumType const* enumType = dynamic_cast<EnumType const*>(magicType->typeArgument()))
						defineExpr(_memberAccess, memberName == "min" ? enumType->minValue() : enumType->maxValue());
				}
				else if (memberName == "interfaceId")
				{
					ContractDefinition const& contract = dynamic_cast<ContractType const&>(*magicType->typeArgument()).contractDefinition();
					defineExpr(_memberAccess, contract.interfaceId());
				}
				else
					// NOTE: supporting name, creationCode, runtimeCode would be easy enough, but the bytes/string they return are not
					//       at all usable in the SMT checker currently
					m_errorReporter.warning(
						7507_error,
						_memberAccess.location(),
						"Assertion checker does not yet support this expression."
					);

			}
		}
		else
			solAssert(false, "");

		return false;
	}
	else if (smt::isNonRecursiveStruct(*exprType))
	{
		memberExpr->accept(*this);
		auto const& symbStruct = dynamic_pointer_cast<smt::SymbolicStructVariable>(m_context.expression(*memberExpr));
		defineExpr(_memberAccess, symbStruct->member(_memberAccess.memberName()));
		return false;
	}
	else if (exprType->category() == Type::Category::TypeType)
	{
		auto const* decl = expressionToDeclaration(*memberExpr);
		if (dynamic_cast<EnumDefinition const*>(decl))
		{
			auto enumType = dynamic_cast<EnumType const*>(accessType);
			solAssert(enumType, "");
			defineExpr(_memberAccess, enumType->memberValue(_memberAccess.memberName()));

			return false;
		}
		else if (dynamic_cast<ContractDefinition const*>(decl))
		{
			if (auto const* var = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
			{
				defineExpr(_memberAccess, currentValue(*var));
				return false;
			}
		}
	}
	else if (exprType->category() == Type::Category::Address)
	{
		memberExpr->accept(*this);
		if (_memberAccess.memberName() == "balance")
		{
			defineExpr(_memberAccess, state().balance(expr(*memberExpr)));
			setSymbolicUnknownValue(*m_context.expression(_memberAccess), m_context);
			m_uninterpretedTerms.insert(&_memberAccess);
			return false;
		}
	}
	else if (exprType->category() == Type::Category::Array)
	{
		memberExpr->accept(*this);
		if (_memberAccess.memberName() == "length")
		{
			auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(*memberExpr));
			solAssert(symbArray, "");
			defineExpr(_memberAccess, symbArray->length());
			m_uninterpretedTerms.insert(&_memberAccess);
			setSymbolicUnknownValue(
				expr(_memberAccess),
				_memberAccess.annotation().type,
				m_context
			);
		}
		return false;
	}
	else if (
		auto const* functionType = dynamic_cast<FunctionType const*>(exprType);
		functionType &&
		_memberAccess.memberName() == "selector" &&
		functionType->hasDeclaration()
	)
	{
		defineExpr(_memberAccess, functionType->externalIdentifier());
		return false;
	}
	else if (exprType->category() == Type::Category::Module)
	{
		if (auto const* var = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
		{
			solAssert(var->isConstant(), "");
			defineExpr(_memberAccess, constantExpr(_memberAccess, *var));
			return false;
		}
	}

	m_errorReporter.warning(
		7650_error,
		_memberAccess.location(),
		"Assertion checker does not yet support this expression."
	);

	return true;
}

void SMTEncoder::endVisit(IndexAccess const& _indexAccess)
{
	createExpr(_indexAccess);

	if (_indexAccess.annotation().type->category() == Type::Category::TypeType)
		return;

	makeOutOfBoundsVerificationTarget(_indexAccess);

	if (auto const* type = dynamic_cast<FixedBytesType const*>(_indexAccess.baseExpression().annotation().type))
	{
		smtutil::Expression base = expr(_indexAccess.baseExpression());

		if (type->numBytes() == 1)
			defineExpr(_indexAccess, base);
		else
		{
			auto [bvSize, isSigned] = smt::typeBvSizeAndSignedness(_indexAccess.baseExpression().annotation().type);
			solAssert(!isSigned, "");
			solAssert(bvSize >= 16, "");
			solAssert(bvSize % 8 == 0, "");

			smtutil::Expression idx = expr(*_indexAccess.indexExpression());

			auto bvBase = smtutil::Expression::int2bv(base, bvSize);
			auto bvShl = smtutil::Expression::int2bv(idx * 8, bvSize);
			auto bvShr = smtutil::Expression::int2bv(bvSize - 8, bvSize);
			auto result = (bvBase << bvShl) >> bvShr;

			auto anyValue = expr(_indexAccess);
			m_context.expression(_indexAccess)->increaseIndex();
			unsigned numBytes = bvSize / 8;
			auto withBound = smtutil::Expression::ite(
				idx < numBytes,
				smtutil::Expression::bv2int(result, false),
				anyValue
			);
			defineExpr(_indexAccess, withBound);
		}
		return;
	}

	shared_ptr<smt::SymbolicVariable> array;
	if (auto const* id = dynamic_cast<Identifier const*>(&_indexAccess.baseExpression()))
	{
		auto varDecl = identifierToVariable(*id);
		solAssert(varDecl, "");
		array = m_context.variable(*varDecl);

		if (varDecl && varDecl->isConstant())
			m_context.addAssertion(currentValue(*varDecl) == expr(*id));
	}
	else
	{
		solAssert(m_context.knownExpression(_indexAccess.baseExpression()), "");
		array = m_context.expression(_indexAccess.baseExpression());
	}

	auto arrayVar = dynamic_pointer_cast<smt::SymbolicArrayVariable>(array);
	solAssert(arrayVar, "");

	Type const* baseType = _indexAccess.baseExpression().annotation().type;
	defineExpr(_indexAccess, smtutil::Expression::select(
		arrayVar->elements(),
		expr(*_indexAccess.indexExpression(), keyType(baseType))
	));
	setSymbolicUnknownValue(
		expr(_indexAccess),
		_indexAccess.annotation().type,
		m_context
	);
	m_uninterpretedTerms.insert(&_indexAccess);
}

void SMTEncoder::endVisit(IndexRangeAccess const& _indexRangeAccess)
{
	createExpr(_indexRangeAccess);
	/// The actual slice is created by CHC which also assigns the length.
}

void SMTEncoder::arrayAssignment()
{
	m_arrayAssignmentHappened = true;
}

void SMTEncoder::indexOrMemberAssignment(Expression const& _expr, smtutil::Expression const& _rightHandSide)
{
	auto toStore = _rightHandSide;
	auto const* lastExpr = &_expr;
	while (true)
	{
		if (auto const* indexAccess = dynamic_cast<IndexAccess const*>(lastExpr))
		{
			auto const& base = indexAccess->baseExpression();
			if (dynamic_cast<Identifier const*>(&base))
				base.accept(*this);

			Type const* baseType = base.annotation().type;
			auto indexExpr = expr(*indexAccess->indexExpression(), keyType(baseType));
			auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(base));
			solAssert(symbArray, "");
			toStore = smtutil::Expression::tuple_constructor(
				smtutil::Expression(make_shared<smtutil::SortSort>(smt::smtSort(*baseType)), baseType->toString(true)),
				{smtutil::Expression::store(symbArray->elements(), indexExpr, toStore), symbArray->length()}
			);
			defineExpr(*indexAccess, smtutil::Expression::select(
				symbArray->elements(),
				indexExpr
			));
			lastExpr = &indexAccess->baseExpression();
		}
		else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(lastExpr))
		{
			auto const& base = memberAccess->expression();
			if (dynamic_cast<Identifier const*>(&base))
				base.accept(*this);

			if (
				auto const* structType = dynamic_cast<StructType const*>(base.annotation().type);
				structType && structType->recursive()
			)
			{
				m_errorReporter.warning(
					4375_error,
					memberAccess->location(),
					"Assertion checker does not support recursive structs."
				);
				return;
			}
			if (auto varDecl = identifierToVariable(*memberAccess))
			{
				if (varDecl->hasReferenceOrMappingType())
					resetReferences(*varDecl);

				assignment(*varDecl, toStore);
				break;
			}

			auto symbStruct = dynamic_pointer_cast<smt::SymbolicStructVariable>(m_context.expression(base));
			solAssert(symbStruct, "");
			symbStruct->assignMember(memberAccess->memberName(), toStore);
			toStore = symbStruct->currentValue();
			defineExpr(*memberAccess, symbStruct->member(memberAccess->memberName()));
			lastExpr = &memberAccess->expression();
		}
		else if (auto const& id = dynamic_cast<Identifier const*>(lastExpr))
		{
			auto varDecl = identifierToVariable(*id);
			solAssert(varDecl, "");

			if (varDecl->hasReferenceOrMappingType())
				resetReferences(*varDecl);

			assignment(*varDecl, toStore);
			defineExpr(*id, currentValue(*varDecl));
			break;
		}
		else
		{
			auto type = lastExpr->annotation().type;
			if (
				dynamic_cast<ReferenceType const*>(type) ||
				dynamic_cast<MappingType const*>(type)
			)
				resetReferences(type);

			assignment(*m_context.expression(*lastExpr), toStore);
			break;
		}
	}
}

void SMTEncoder::arrayPush(FunctionCall const& _funCall)
{
	auto memberAccess = dynamic_cast<MemberAccess const*>(&_funCall.expression());
	solAssert(memberAccess, "");
	auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
	solAssert(symbArray, "");
	auto oldLength = symbArray->length();
	m_context.addAssertion(oldLength >= 0);
	// Real world assumption: the array length is assumed to not overflow.
	// This assertion guarantees that both the current and updated lengths have the above property.
	m_context.addAssertion(oldLength + 1 < (smt::maxValue(*TypeProvider::uint256()) - 1));

	auto const& arguments = _funCall.arguments();
	auto arrayType = dynamic_cast<ArrayType const*>(symbArray->type());
	solAssert(arrayType, "");
	auto elementType = arrayType->baseType();
	smtutil::Expression element = arguments.empty() ?
		smt::zeroValue(elementType) :
		expr(*arguments.front(), elementType);
	smtutil::Expression store = smtutil::Expression::store(
		symbArray->elements(),
		oldLength,
		element
	);
	symbArray->increaseIndex();
	m_context.addAssertion(symbArray->elements() == store);
	m_context.addAssertion(symbArray->length() == oldLength + 1);

	if (arguments.empty())
		defineExpr(_funCall, element);

	assignment(memberAccess->expression(), symbArray->currentValue());
}

void SMTEncoder::arrayPop(FunctionCall const& _funCall)
{
	auto memberAccess = dynamic_cast<MemberAccess const*>(cleanExpression(_funCall.expression()));
	solAssert(memberAccess, "");
	auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
	solAssert(symbArray, "");

	makeArrayPopVerificationTarget(_funCall);

	auto oldElements = symbArray->elements();
	auto oldLength = symbArray->length();

	symbArray->increaseIndex();
	m_context.addAssertion(symbArray->elements() == oldElements);
	auto newLength = smtutil::Expression::ite(
		oldLength > 0,
		oldLength - 1,
		0
	);
	m_context.addAssertion(symbArray->length() == newLength);

	assignment(memberAccess->expression(), symbArray->currentValue());
}

void SMTEncoder::defineGlobalVariable(string const& _name, Expression const& _expr, bool _increaseIndex)
{
	if (!m_context.knownGlobalSymbol(_name))
	{
		bool abstract = m_context.createGlobalSymbol(_name, _expr);
		if (abstract)
			m_errorReporter.warning(
				1695_error,
				_expr.location(),
				"Assertion checker does not yet support this global variable."
			);
	}
	else if (_increaseIndex)
		m_context.globalSymbol(_name)->increaseIndex();
	// The default behavior is not to increase the index since
	// most of the global values stay the same throughout a tx.
	if (isSupportedType(*_expr.annotation().type))
		defineExpr(_expr, m_context.globalSymbol(_name)->currentValue());
}

bool SMTEncoder::shortcutRationalNumber(Expression const& _expr)
{
	RationalNumberType const* rationalType = isConstant(_expr);
	if (!rationalType)
		return false;

	if (rationalType->isNegative())
		defineExpr(_expr, smtutil::Expression(u2s(rationalType->literalValue(nullptr))));
	else
		defineExpr(_expr, smtutil::Expression(rationalType->literalValue(nullptr)));
	return true;
}

void SMTEncoder::arithmeticOperation(BinaryOperation const& _op)
{
	auto type = _op.annotation().commonType;
	solAssert(type, "");
	solAssert(type->category() == Type::Category::Integer || type->category() == Type::Category::FixedPoint, "");
	switch (_op.getOperator())
	{
	case Token::Add:
	case Token::Sub:
	case Token::Mul:
	case Token::Div:
	case Token::Mod:
	{
		auto values = arithmeticOperation(
			_op.getOperator(),
			expr(_op.leftExpression()),
			expr(_op.rightExpression()),
			_op.annotation().commonType,
			_op
		);
		defineExpr(_op, values.first);
		break;
	}
	default:
		m_errorReporter.warning(
			5188_error,
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
	}
}

pair<smtutil::Expression, smtutil::Expression> SMTEncoder::arithmeticOperation(
	Token _op,
	smtutil::Expression const& _left,
	smtutil::Expression const& _right,
	Type const* _commonType,
	Expression const& _operation
)
{
	static set<Token> validOperators{
		Token::Add,
		Token::Sub,
		Token::Mul,
		Token::Div,
		Token::Mod
	};
	solAssert(validOperators.count(_op), "");
	solAssert(_commonType, "");
	solAssert(
		_commonType->category() == Type::Category::Integer || _commonType->category() == Type::Category::FixedPoint,
		""
	);

	IntegerType const* intType = nullptr;
	if (auto type = dynamic_cast<IntegerType const*>(_commonType))
		intType = type;
	else
		intType = TypeProvider::uint256();

	auto valueUnbounded = [&]() -> smtutil::Expression {
		switch (_op)
		{
		case Token::Add: return _left + _right;
		case Token::Sub: return _left - _right;
		case Token::Mul: return _left * _right;
		case Token::Div: return divModWithSlacks(_left, _right, *intType).first;
		case Token::Mod: return divModWithSlacks(_left, _right, *intType).second;
		default: solAssert(false, "");
		}
	}();

	if (m_checked)
		return {valueUnbounded, valueUnbounded};

	if (_op == Token::Div || _op == Token::Mod)
	{
		// mod and unsigned division never underflow/overflow
		if (_op == Token::Mod || !intType->isSigned())
			return {valueUnbounded, valueUnbounded};

		// The only case where division overflows is
		// - type is signed
		// - LHS is type.min
		// - RHS is -1
		// the result is then -(type.min), which wraps back to type.min
		smtutil::Expression maxLeft = _left == smt::minValue(*intType);
		smtutil::Expression minusOneRight = _right == numeric_limits<size_t >::max();
		smtutil::Expression wrap = smtutil::Expression::ite(maxLeft && minusOneRight, smt::minValue(*intType), valueUnbounded);
		return {wrap, valueUnbounded};
	}

	auto symbMin = smt::minValue(*intType);
	auto symbMax = smt::maxValue(*intType);

	smtutil::Expression intValueRange = (0 - symbMin) + symbMax + 1;
	string suffix = to_string(_operation.id()) + "_" + to_string(m_context.newUniqueId());
	smt::SymbolicIntVariable k(intType, intType, "k_" + suffix, m_context);
	smt::SymbolicIntVariable m(intType, intType, "m_" + suffix, m_context);

	// To wrap around valueUnbounded in case of overflow or underflow, we replace it with a k, given:
	// 1. k + m * intValueRange = valueUnbounded
	// 2. k is in range of the desired integer type
	auto wrap = k.currentValue();
	m_context.addAssertion(valueUnbounded == (k.currentValue() + intValueRange * m.currentValue()));
	m_context.addAssertion(k.currentValue() >= symbMin);
	m_context.addAssertion(k.currentValue() <= symbMax);

	// TODO this could be refined:
	// for unsigned types it's enough to check only the upper bound.
	auto value = smtutil::Expression::ite(
		valueUnbounded > symbMax,
		wrap,
		smtutil::Expression::ite(
			valueUnbounded < symbMin,
			wrap,
			valueUnbounded
		)
	);

	return {value, valueUnbounded};
}

smtutil::Expression SMTEncoder::bitwiseOperation(
	Token _op,
	smtutil::Expression const& _left,
	smtutil::Expression const& _right,
	Type const* _commonType
)
{
	static set<Token> validOperators{
		Token::BitAnd,
		Token::BitOr,
		Token::BitXor,
		Token::SHL,
		Token::SHR,
		Token::SAR
	};
	solAssert(validOperators.count(_op), "");
	solAssert(_commonType, "");

	auto [bvSize, isSigned] = smt::typeBvSizeAndSignedness(_commonType);

	auto bvLeft = smtutil::Expression::int2bv(_left, bvSize);
	auto bvRight = smtutil::Expression::int2bv(_right, bvSize);

	optional<smtutil::Expression> result;
	switch (_op)
	{
		case Token::BitAnd:
			result = bvLeft & bvRight;
			break;
		case Token::BitOr:
			result = bvLeft | bvRight;
			break;
		case Token::BitXor:
			result = bvLeft ^ bvRight;
			break;
		case Token::SHL:
			result = bvLeft << bvRight;
			break;
		case Token::SHR:
			result = bvLeft >> bvRight;
			break;
		case Token::SAR:
			result = isSigned ?
				smtutil::Expression::ashr(bvLeft, bvRight) :
				bvLeft >> bvRight;
			break;
		default:
			solAssert(false, "");
	}

	solAssert(result.has_value(), "");
	return smtutil::Expression::bv2int(*result, isSigned);
}

void SMTEncoder::compareOperation(BinaryOperation const& _op)
{
	auto commonType = _op.annotation().commonType;
	solAssert(commonType, "");

	if (isSupportedType(*commonType))
	{
		smtutil::Expression left(expr(_op.leftExpression(), commonType));
		smtutil::Expression right(expr(_op.rightExpression(), commonType));
		Token op = _op.getOperator();
		shared_ptr<smtutil::Expression> value;
		if (smt::isNumber(*commonType))
		{
			value = make_shared<smtutil::Expression>(
				op == Token::Equal ? (left == right) :
				op == Token::NotEqual ? (left != right) :
				op == Token::LessThan ? (left < right) :
				op == Token::LessThanOrEqual ? (left <= right) :
				op == Token::GreaterThan ? (left > right) :
				/*op == Token::GreaterThanOrEqual*/ (left >= right)
			);
		}
		else // Bool
		{
			solUnimplementedAssert(smt::isBool(*commonType), "Operation not yet supported");
			value = make_shared<smtutil::Expression>(
				op == Token::Equal ? (left == right) :
				/*op == Token::NotEqual*/ (left != right)
			);
		}
		// TODO: check that other values for op are not possible.
		defineExpr(_op, *value);
	}
	else
		m_errorReporter.warning(
			7229_error,
			_op.location(),
			"Assertion checker does not yet implement the type " + _op.annotation().commonType->toString() + " for comparisons"
		);
}

void SMTEncoder::booleanOperation(BinaryOperation const& _op)
{
	solAssert(_op.getOperator() == Token::And || _op.getOperator() == Token::Or, "");
	solAssert(_op.annotation().commonType, "");
	solAssert(_op.annotation().commonType->category() == Type::Category::Bool, "");
	// @TODO check that both of them are not constant
	_op.leftExpression().accept(*this);
	if (_op.getOperator() == Token::And)
	{
		auto indicesAfterSecond = visitBranch(&_op.rightExpression(), expr(_op.leftExpression())).first;
		mergeVariables(!expr(_op.leftExpression()), copyVariableIndices(), indicesAfterSecond);
		defineExpr(_op, expr(_op.leftExpression()) && expr(_op.rightExpression()));
	}
	else
	{
		auto indicesAfterSecond = visitBranch(&_op.rightExpression(), !expr(_op.leftExpression())).first;
		mergeVariables(expr(_op.leftExpression()), copyVariableIndices(), indicesAfterSecond);
		defineExpr(_op, expr(_op.leftExpression()) || expr(_op.rightExpression()));
	}
}

void SMTEncoder::bitwiseOperation(BinaryOperation const& _op)
{
	auto op = _op.getOperator();
	solAssert(TokenTraits::isBitOp(op) || TokenTraits::isShiftOp(op), "");
	auto commonType = _op.annotation().commonType;
	solAssert(commonType, "");

	defineExpr(_op, bitwiseOperation(
		_op.getOperator(),
		expr(_op.leftExpression(), commonType),
		expr(_op.rightExpression(), commonType),
		commonType
	));
}

void SMTEncoder::bitwiseNotOperation(UnaryOperation const& _op)
{
	solAssert(_op.getOperator() == Token::BitNot, "");

	auto [bvSize, isSigned] = smt::typeBvSizeAndSignedness(_op.annotation().type);

	auto bvOperand = smtutil::Expression::int2bv(expr(_op.subExpression(), _op.annotation().type), bvSize);
	defineExpr(_op, smtutil::Expression::bv2int(~bvOperand, isSigned));
}

pair<smtutil::Expression, smtutil::Expression> SMTEncoder::divModWithSlacks(
	smtutil::Expression _left,
	smtutil::Expression _right,
	IntegerType const& _type
)
{
	if (m_settings.divModNoSlacks)
		return {_left / _right, _left % _right};

	IntegerType const* intType = &_type;
	string suffix = "div_mod_" + to_string(m_context.newUniqueId());
	smt::SymbolicIntVariable dSymb(intType, intType, "d_" + suffix, m_context);
	smt::SymbolicIntVariable rSymb(intType, intType, "r_" + suffix, m_context);
	auto d = dSymb.currentValue();
	auto r = rSymb.currentValue();

	// x / y = d and x % y = r iff d * y + r = x and
	// either x >= 0 and 0 <= r < abs(y) (or just 0 <= r < y for unsigned)
	// or     x < 0 and -abs(y) < r <= 0
	m_context.addAssertion(((d * _right) + r) == _left);
	if (_type.isSigned())
		m_context.addAssertion(
			(_left >= 0 && 0 <= r && (_right == 0 || r < smtutil::abs(_right))) ||
			(_left < 0 && ((_right == 0 || 0 - smtutil::abs(_right) < r) && r <= 0))
		);
	else // unsigned version
		m_context.addAssertion(0 <= r && (_right == 0 || r < _right));

	auto divResult = smtutil::Expression::ite(_right == 0, 0, d);
	auto modResult = smtutil::Expression::ite(_right == 0, 0, r);
	return {divResult, modResult};
}

void SMTEncoder::assignment(Expression const& _left, smtutil::Expression const& _right)
{
	assignment(_left, _right, _left.annotation().type);
}

void SMTEncoder::assignment(
	Expression const& _left,
	smtutil::Expression const& _right,
	Type const* _type
)
{
	solAssert(
		_left.annotation().type->category() != Type::Category::Tuple,
		"Tuple assignments should be handled by tupleAssignment."
	);

	Expression const* left = cleanExpression(_left);

	if (!isSupportedType(*_type))
	{
		// Give it a new index anyway to keep the SSA scheme sound.
		if (auto varDecl = identifierToVariable(*left))
			m_context.newValue(*varDecl);
	}
	else if (auto varDecl = identifierToVariable(*left))
	{
		if (varDecl->hasReferenceOrMappingType())
			resetReferences(*varDecl);
		assignment(*varDecl, _right);
	}
	else if (
		dynamic_cast<IndexAccess const*>(left) ||
		dynamic_cast<MemberAccess const*>(left)
	)
		indexOrMemberAssignment(*left, _right);
	else if (auto const* funCall = dynamic_cast<FunctionCall const*>(left))
	{
		if (auto funType = dynamic_cast<FunctionType const*>(funCall->expression().annotation().type))
		{
			if (funType->kind() == FunctionType::Kind::ArrayPush)
			{
				auto memberAccess = dynamic_cast<MemberAccess const*>(&funCall->expression());
				solAssert(memberAccess, "");
				auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
				solAssert(symbArray, "");

				auto oldLength = symbArray->length();
				auto store = smtutil::Expression::store(
					symbArray->elements(),
					symbArray->length() - 1,
					_right
				);
				symbArray->increaseIndex();
				m_context.addAssertion(symbArray->elements() == store);
				m_context.addAssertion(symbArray->length() == oldLength);
				assignment(memberAccess->expression(), symbArray->currentValue());
			}
			else if (funType->kind() == FunctionType::Kind::Internal)
			{
				for (auto type: replaceUserTypes(funType->returnParameterTypes()))
					if (type->category() == Type::Category::Mapping || dynamic_cast<ReferenceType const*>(type))
						resetReferences(type);
			}
		}
	}
	else
		solAssert(false, "");
}

void SMTEncoder::tupleAssignment(Expression const& _left, Expression const& _right)
{
	auto lTuple = dynamic_cast<TupleExpression const*>(innermostTuple(_left));
	solAssert(lTuple, "");
	Expression const* right = innermostTuple(_right);

	auto const& lComponents = lTuple->components();

	// If both sides are tuple expressions, we individually and potentially
	// recursively assign each pair of components.
	// This is because of potential type conversion.
	if (auto rTuple = dynamic_cast<TupleExpression const*>(right))
	{
		auto const& rComponents = rTuple->components();
		solAssert(lComponents.size() == rComponents.size(), "");
		for (unsigned i = 0; i < lComponents.size(); ++i)
		{
			if (!lComponents.at(i) || !rComponents.at(i))
				continue;
			auto const& lExpr = *lComponents.at(i);
			auto const& rExpr = *rComponents.at(i);
			if (lExpr.annotation().type->category() == Type::Category::Tuple)
				tupleAssignment(lExpr, rExpr);
			else
			{
				auto type = lExpr.annotation().type;
				assignment(lExpr, expr(rExpr, type), type);
			}
		}
	}
	else
	{
		auto rType = dynamic_cast<TupleType const*>(right->annotation().type);
		solAssert(rType, "");

		auto const& rComponents = rType->components();
		solAssert(lComponents.size() == rComponents.size(), "");

		auto symbRight = expr(*right);
		solAssert(symbRight.sort->kind == smtutil::Kind::Tuple, "");

		for (unsigned i = 0; i < lComponents.size(); ++i)
			if (auto component = lComponents.at(i); component && rComponents.at(i))
				assignment(*component, smtutil::Expression::tuple_get(symbRight, i), component->annotation().type);
	}
}

smtutil::Expression SMTEncoder::compoundAssignment(Assignment const& _assignment)
{
	static map<Token, Token> const compoundToArithmetic{
		{Token::AssignAdd, Token::Add},
		{Token::AssignSub, Token::Sub},
		{Token::AssignMul, Token::Mul},
		{Token::AssignDiv, Token::Div},
		{Token::AssignMod, Token::Mod}
	};
	static map<Token, Token> const compoundToBitwise{
		{Token::AssignBitAnd, Token::BitAnd},
		{Token::AssignBitOr, Token::BitOr},
		{Token::AssignBitXor, Token::BitXor},
		{Token::AssignShl, Token::SHL},
		{Token::AssignShr, Token::SHR},
		{Token::AssignSar, Token::SAR}
	};
	Token op = _assignment.assignmentOperator();
	solAssert(compoundToArithmetic.count(op) || compoundToBitwise.count(op), "");

	auto decl = identifierToVariable(_assignment.leftHandSide());

	if (compoundToBitwise.count(op))
		return bitwiseOperation(
			compoundToBitwise.at(op),
			decl ? currentValue(*decl) : expr(_assignment.leftHandSide(), _assignment.annotation().type),
			expr(_assignment.rightHandSide(), _assignment.annotation().type),
			_assignment.annotation().type
		);

	auto values = arithmeticOperation(
		compoundToArithmetic.at(op),
		decl ? currentValue(*decl) : expr(_assignment.leftHandSide(), _assignment.annotation().type),
		expr(_assignment.rightHandSide(), _assignment.annotation().type),
		_assignment.annotation().type,
		_assignment
	);
	return values.first;
}

void SMTEncoder::expressionToTupleAssignment(vector<shared_ptr<VariableDeclaration>> const& _variables, Expression const& _rhs)
{
	auto symbolicVar = m_context.expression(_rhs);
	if (_variables.size() > 1)
	{
		auto symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(symbolicVar);
		solAssert(symbTuple, "");
		auto const& symbComponents = symbTuple->components();
		solAssert(symbComponents.size() == _variables.size(), "");
		auto tupleType = dynamic_cast<TupleType const*>(_rhs.annotation().type);
		solAssert(tupleType, "");
		auto const& typeComponents = tupleType->components();
		solAssert(typeComponents.size() == symbComponents.size(), "");
		for (unsigned i = 0; i < symbComponents.size(); ++i)
		{
			auto param = _variables.at(i);
			if (param)
			{
				solAssert(m_context.knownVariable(*param), "");
				assignment(*param, symbTuple->component(i, typeComponents[i], param->type()));
			}
		}
	}
	else if (_variables.size() == 1)
	{
		auto const& var = *_variables.front();
		solAssert(m_context.knownVariable(var), "");
		assignment(var, _rhs);
	}
}

void SMTEncoder::assignment(VariableDeclaration const& _variable, Expression const& _value)
{
	// In general, at this point, the SMT sorts of _variable and _value are the same,
	// even if there is implicit conversion.
	// This is a special case where the SMT sorts are different.
	// For now we are unaware of other cases where this happens, but if they do appear
	// we should extract this into an `implicitConversion` function.
	assignment(_variable, expr(_value, _variable.type()));
}

void SMTEncoder::assignment(VariableDeclaration const& _variable, smtutil::Expression const& _value)
{
	Type const* type = _variable.type();
	if (type->category() == Type::Category::Mapping)
		arrayAssignment();
	assignment(*m_context.variable(_variable), _value);
}

void SMTEncoder::assignment(smt::SymbolicVariable& _symVar, smtutil::Expression const& _value)
{
	m_context.addAssertion(_symVar.increaseIndex() == _value);
}

pair<SMTEncoder::VariableIndices, smtutil::Expression> SMTEncoder::visitBranch(
	ASTNode const* _statement,
	smtutil::Expression _condition
)
{
	return visitBranch(_statement, &_condition);
}

pair<SMTEncoder::VariableIndices, smtutil::Expression> SMTEncoder::visitBranch(
	ASTNode const* _statement,
	smtutil::Expression const* _condition
)
{
	auto indicesBeforeBranch = copyVariableIndices();
	if (_condition)
		pushPathCondition(*_condition);
	_statement->accept(*this);
	auto pathConditionOnExit = currentPathConditions();
	if (_condition)
		popPathCondition();
	auto indicesAfterBranch = copyVariableIndices();
	resetVariableIndices(indicesBeforeBranch);
	return {indicesAfterBranch, pathConditionOnExit};
}

void SMTEncoder::initializeFunctionCallParameters(CallableDeclaration const& _function, vector<smtutil::Expression> const& _callArgs)
{
	auto const& funParams = _function.parameters();
	solAssert(funParams.size() == _callArgs.size(), "");
	for (unsigned i = 0; i < funParams.size(); ++i)
		if (createVariable(*funParams[i]))
		{
			m_context.addAssertion(_callArgs[i] == m_context.newValue(*funParams[i]));
			if (funParams[i]->annotation().type->category() == Type::Category::Mapping)
				m_arrayAssignmentHappened = true;
		}

	vector<VariableDeclaration const*> localVars;
	if (auto const* fun = dynamic_cast<FunctionDefinition const*>(&_function))
		localVars = localVariablesIncludingModifiers(*fun, m_currentContract);
	else
		localVars = _function.localVariables();
	for (auto const& variable: localVars)
		if (createVariable(*variable))
		{
			m_context.newValue(*variable);
			m_context.setZeroValue(*variable);
		}

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			if (createVariable(*retParam))
			{
				m_context.newValue(*retParam);
				m_context.setZeroValue(*retParam);
			}
}

void SMTEncoder::createStateVariables(ContractDefinition const& _contract)
{
	for (auto var: stateVariablesIncludingInheritedAndPrivate(_contract))
		createVariable(*var);
}

void SMTEncoder::initializeStateVariables(ContractDefinition const& _contract)
{
	for (auto var: _contract.stateVariables())
	{
		solAssert(m_context.knownVariable(*var), "");
		m_context.setZeroValue(*var);
	}

	for (auto var: _contract.stateVariables())
		if (var->value())
		{
			var->value()->accept(*this);
			assignment(*var, *var->value());
		}
}

void SMTEncoder::createLocalVariables(FunctionDefinition const& _function)
{
	for (auto const& variable: localVariablesIncludingModifiers(_function, m_currentContract))
		createVariable(*variable);

	for (auto const& param: _function.parameters())
		createVariable(*param);

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			createVariable(*retParam);
}

void SMTEncoder::initializeLocalVariables(FunctionDefinition const& _function)
{
	for (auto const& variable: localVariablesIncludingModifiers(_function, m_currentContract))
	{
		solAssert(m_context.knownVariable(*variable), "");
		m_context.setZeroValue(*variable);
	}

	for (auto const& param: _function.parameters())
	{
		solAssert(m_context.knownVariable(*param), "");
		m_context.setUnknownValue(*param);
	}

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
		{
			solAssert(m_context.knownVariable(*retParam), "");
			m_context.setZeroValue(*retParam);
		}
}

void SMTEncoder::resetStateVariables()
{
	m_context.resetVariables([&](VariableDeclaration const& _variable) { return _variable.isStateVariable(); });
}

void SMTEncoder::resetMemoryVariables()
{
	m_context.resetVariables([&](VariableDeclaration const& _variable) {
		return _variable.referenceLocation() == VariableDeclaration::Location::Memory;
	});
}

void SMTEncoder::resetStorageVariables()
{
	m_context.resetVariables([&](VariableDeclaration const& _variable) {
		return _variable.referenceLocation() == VariableDeclaration::Location::Storage || _variable.isStateVariable();
	});
}

void SMTEncoder::resetBalances()
{
	state().newBalances();
}

void SMTEncoder::resetReferences(VariableDeclaration const& _varDecl)
{
	m_context.resetVariables([&](VariableDeclaration const& _var) {
		if (_var == _varDecl)
			return false;

		// If both are state variables no need to clear knowledge.
		if (_var.isStateVariable() && _varDecl.isStateVariable())
			return false;

		return sameTypeOrSubtype(_var.type(), _varDecl.type());
	});
}

void SMTEncoder::resetReferences(Type const* _type)
{
	m_context.resetVariables([&](VariableDeclaration const& _var) {
		return sameTypeOrSubtype(_var.type(), _type);
	});
}

bool SMTEncoder::sameTypeOrSubtype(Type const* _a, Type const* _b)
{
	bool foundSame = false;

	solidity::util::BreadthFirstSearch<Type const*> bfs{{_a}};
	bfs.run([&](auto _type, auto&& _addChild) {
		if (*typeWithoutPointer(_b) == *typeWithoutPointer(_type))
		{
			foundSame = true;
			bfs.abort();
		}
		if (auto const* mapType = dynamic_cast<MappingType const*>(_type))
			_addChild(mapType->valueType());
		else if (auto const* arrayType = dynamic_cast<ArrayType const*>(_type))
			_addChild(arrayType->baseType());
		else if (auto const* structType = dynamic_cast<StructType const*>(_type))
			for (auto const& member: structType->nativeMembers(nullptr))
				_addChild(member.type);
	});

	return foundSame;
}

bool SMTEncoder::isSupportedType(Type const& _type) const
{
	return smt::isSupportedType(*underlyingType(&_type));
}

Type const* SMTEncoder::typeWithoutPointer(Type const* _type)
{
	if (auto refType = dynamic_cast<ReferenceType const*>(_type))
		return TypeProvider::withLocationIfReference(refType->location(), _type);
	return _type;
}

void SMTEncoder::mergeVariables(smtutil::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse)
{
	for (auto const& entry: _indicesEndTrue)
	{
		VariableDeclaration const* var = entry.first;
		auto trueIndex = entry.second;
		if (_indicesEndFalse.count(var) && _indicesEndFalse.at(var) != trueIndex)
		{
			m_context.addAssertion(m_context.newValue(*var) == smtutil::Expression::ite(
				_condition,
				valueAtIndex(*var, trueIndex),
				valueAtIndex(*var, _indicesEndFalse.at(var)))
			);
		}
	}
}

smtutil::Expression SMTEncoder::currentValue(VariableDeclaration const& _decl) const
{
	solAssert(m_context.knownVariable(_decl), "");
	return m_context.variable(_decl)->currentValue();
}

smtutil::Expression SMTEncoder::valueAtIndex(VariableDeclaration const& _decl, unsigned _index) const
{
	solAssert(m_context.knownVariable(_decl), "");
	return m_context.variable(_decl)->valueAtIndex(_index);
}

bool SMTEncoder::createVariable(VariableDeclaration const& _varDecl)
{
	if (m_context.knownVariable(_varDecl))
		return true;
	bool abstract = m_context.createVariable(_varDecl);
	if (abstract)
	{
		m_errorReporter.warning(
			8115_error,
			_varDecl.location(),
			"Assertion checker does not yet support the type of this variable."
		);
		return false;
	}
	return true;
}

smtutil::Expression SMTEncoder::expr(Expression const& _e, Type const* _targetType)
{
	if (!m_context.knownExpression(_e))
	{
		m_errorReporter.warning(6031_error, _e.location(), "Internal error: Expression undefined for SMT solver." );
		createExpr(_e);
	}

	return m_context.expression(_e)->currentValue(underlyingType(_targetType));
}

void SMTEncoder::createExpr(Expression const& _e)
{
	bool abstract = m_context.createExpression(_e);
	if (abstract)
		m_errorReporter.warning(
			8364_error,
			_e.location(),
			"Assertion checker does not yet implement type " + _e.annotation().type->toString()
		);
}

void SMTEncoder::defineExpr(Expression const& _e, smtutil::Expression _value)
{
	auto type = _e.annotation().type;
	createExpr(_e);
	solAssert(_value.sort->kind != smtutil::Kind::Function, "Equality operator applied to type that is not fully supported");
	if (!smt::isInaccessibleDynamic(*type))
		m_context.addAssertion(expr(_e) == _value);

	if (m_checked && smt::isNumber(*type))
		m_context.addAssertion(smtutil::Expression::implies(
			currentPathConditions(),
			smt::symbolicUnknownConstraints(expr(_e), type)
		));
}

void SMTEncoder::defineExpr(Expression const& _e, vector<optional<smtutil::Expression>> const& _values)
{
	if (_values.size() == 1 && _values.front())
	{
		defineExpr(_e, *_values.front());
		return;
	}
	auto const& symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(_e));
	solAssert(symbTuple, "");
	symbTuple->increaseIndex();
	auto const& symbComponents = symbTuple->components();
	solAssert(symbComponents.size() == _values.size(), "");
	auto tupleType = dynamic_cast<TupleType const*>(_e.annotation().type);
	solAssert(tupleType, "");
	solAssert(tupleType->components().size() == symbComponents.size(), "");
	for (unsigned i = 0; i < symbComponents.size(); ++i)
		if (_values[i] && !smt::isInaccessibleDynamic(*tupleType->components()[i]))
			m_context.addAssertion(symbTuple->component(i) == *_values[i]);
}

void SMTEncoder::popPathCondition()
{
	solAssert(m_pathConditions.size() > 0, "Cannot pop path condition, empty.");
	m_pathConditions.pop_back();
}

void SMTEncoder::pushPathCondition(smtutil::Expression const& _e)
{
	m_pathConditions.push_back(currentPathConditions() && _e);
}

void SMTEncoder::setPathCondition(smtutil::Expression const& _e)
{
	if (m_pathConditions.empty())
		m_pathConditions.push_back(_e);
	else
		m_pathConditions.back() = _e;
}

smtutil::Expression SMTEncoder::currentPathConditions()
{
	if (m_pathConditions.empty())
		return smtutil::Expression(true);
	return m_pathConditions.back();
}

SecondarySourceLocation SMTEncoder::callStackMessage(vector<CallStackEntry> const& _callStack)
{
	SecondarySourceLocation callStackLocation;
	solAssert(!_callStack.empty(), "");
	callStackLocation.append("Callstack:", SourceLocation());
	for (auto const& call: _callStack | ranges::views::reverse)
		if (call.second)
			callStackLocation.append("", call.second->location());
	return callStackLocation;
}

pair<CallableDeclaration const*, ASTNode const*> SMTEncoder::popCallStack()
{
	solAssert(!m_callStack.empty(), "");
	auto lastCalled = m_callStack.back();
	m_callStack.pop_back();
	return lastCalled;
}

void SMTEncoder::pushCallStack(CallStackEntry _entry)
{
	m_callStack.push_back(_entry);
}

void SMTEncoder::addPathImpliedExpression(smtutil::Expression const& _e)
{
	m_context.addAssertion(smtutil::Expression::implies(currentPathConditions(), _e));
}

bool SMTEncoder::isRootFunction()
{
	return m_callStack.size() == 1;
}

bool SMTEncoder::visitedFunction(FunctionDefinition const* _funDef)
{
	for (auto const& call: m_callStack)
		if (call.first == _funDef)
			return true;
	return false;
}

ContractDefinition const* SMTEncoder::currentScopeContract()
{
	for (auto&& f: m_callStack | ranges::views::reverse | ranges::views::keys)
		if (auto fun = dynamic_cast<FunctionDefinition const*>(f))
			return fun->annotation().contract;
	return nullptr;
}

SMTEncoder::VariableIndices SMTEncoder::copyVariableIndices()
{
	VariableIndices indices;
	for (auto const& var: m_context.variables())
		indices.emplace(var.first, var.second->index());
	return indices;
}

void SMTEncoder::resetVariableIndices(VariableIndices const& _indices)
{
	for (auto const& var: _indices)
		m_context.variable(*var.first)->setIndex(var.second);
}

void SMTEncoder::clearIndices(ContractDefinition const* _contract, FunctionDefinition const* _function)
{
	solAssert(_contract, "");
	for (auto var: stateVariablesIncludingInheritedAndPrivate(*_contract))
		m_context.variable(*var)->resetIndex();
	if (_function)
	{
		for (auto const& var: _function->parameters() + _function->returnParameters())
			m_context.variable(*var)->resetIndex();
		for (auto const& var: localVariablesIncludingModifiers(*_function, _contract))
			m_context.variable(*var)->resetIndex();
	}
	state().reset();
}

Expression const* SMTEncoder::leftmostBase(IndexAccess const& _indexAccess)
{
	Expression const* base = &_indexAccess.baseExpression();
	while (auto access = dynamic_cast<IndexAccess const*>(base))
		base = &access->baseExpression();
	return base;
}

Type const* SMTEncoder::keyType(Type const* _type)
{
	if (auto const* mappingType = dynamic_cast<MappingType const*>(_type))
		return mappingType->keyType();
	if (
		dynamic_cast<ArrayType const*>(_type) ||
		dynamic_cast<ArraySliceType const*>(_type)
	)
		return TypeProvider::uint256();
	else
		solAssert(false, "");
}

Expression const* SMTEncoder::innermostTuple(Expression const& _expr)
{
	auto const* tuple = dynamic_cast<TupleExpression const*>(&_expr);
	if (!tuple || tuple->isInlineArray())
		return &_expr;

	Expression const* expr = tuple;
	while (tuple && !tuple->isInlineArray() && tuple->components().size() == 1)
	{
		expr = tuple->components().front().get();
		tuple = dynamic_cast<TupleExpression const*>(expr);
	}
	solAssert(expr, "");
	return expr;
}

Type const* SMTEncoder::underlyingType(Type const* _type)
{
	if (auto userType = dynamic_cast<UserDefinedValueType const*>(_type))
		_type = &userType->underlyingType();
	return _type;
}

TypePointers SMTEncoder::replaceUserTypes(TypePointers const& _types)
{
	return applyMap(_types, [](auto _type) {
		if (auto userType = dynamic_cast<UserDefinedValueType const*>(_type))
			return &userType->underlyingType();
		return _type;
	});
}

pair<Expression const*, FunctionCallOptions const*> SMTEncoder::functionCallExpression(FunctionCall const& _funCall)
{
	Expression const* callExpr = &_funCall.expression();
	auto const* callOptions = dynamic_cast<FunctionCallOptions const*>(callExpr);
	if (callOptions)
		callExpr = &callOptions->expression();

	return {callExpr, callOptions};
}

Expression const* SMTEncoder::cleanExpression(Expression const& _expr)
{
	auto const* expr = &_expr;
	if (auto const* tuple = dynamic_cast<TupleExpression const*>(expr))
		return cleanExpression(*innermostTuple(*tuple));
	if (auto const* functionCall = dynamic_cast<FunctionCall const*>(expr))
		if (*functionCall->annotation().kind == FunctionCallKind::TypeConversion)
		{
			auto typeType = dynamic_cast<TypeType const*>(functionCall->expression().annotation().type);
			solAssert(typeType, "");
			if (auto const* arrayType = dynamic_cast<ArrayType const*>(typeType->actualType()))
				if (arrayType->isByteArrayOrString())
				{
					// this is a cast to `bytes`
					solAssert(functionCall->arguments().size() == 1, "");
					Expression const& arg = *functionCall->arguments()[0];
					if (
						auto const* argArrayType = dynamic_cast<ArrayType const*>(arg.annotation().type);
						argArrayType && argArrayType->isByteArrayOrString()
					)
						return cleanExpression(arg);
				}
		}
	solAssert(expr, "");
	return expr;
}

set<VariableDeclaration const*> SMTEncoder::touchedVariables(ASTNode const& _node)
{
	vector<CallableDeclaration const*> callStack;
	for (auto const& call: m_callStack)
		callStack.push_back(call.first);
	return m_variableUsage.touchedVariables(_node, callStack);
}

Declaration const* SMTEncoder::expressionToDeclaration(Expression const& _expr) const
{
	if (auto const* identifier = dynamic_cast<Identifier const*>(&_expr))
		return identifier->annotation().referencedDeclaration;
	if (auto const* outerMemberAccess = dynamic_cast<MemberAccess const*>(&_expr))
		return outerMemberAccess->annotation().referencedDeclaration;
	return nullptr;
}

VariableDeclaration const* SMTEncoder::identifierToVariable(Expression const& _expr) const
{
	// We do not use `expressionToDeclaration` here because we are not interested in
	// struct.field, for example.
	if (auto const* identifier = dynamic_cast<Identifier const*>(&_expr))
		if (auto const* varDecl = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration))
		{
			solAssert(m_context.knownVariable(*varDecl), "");
			return varDecl;
		}
	// But we are interested in "contract.var", because that is the same as just "var".
	if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(&_expr))
		if (dynamic_cast<ContractDefinition const*>(expressionToDeclaration(memberAccess->expression())))
			if (auto const* varDecl = dynamic_cast<VariableDeclaration const*>(memberAccess->annotation().referencedDeclaration))
			{
				solAssert(m_context.knownVariable(*varDecl), "");
				return varDecl;
			}

	return nullptr;
}

MemberAccess const* SMTEncoder::isEmptyPush(Expression const& _expr) const
{
	if (
		auto const* funCall = dynamic_cast<FunctionCall const*>(&_expr);
		funCall && funCall->arguments().empty()
	)
	{
		auto const& funType = dynamic_cast<FunctionType const&>(*funCall->expression().annotation().type);
		if (funType.kind() == FunctionType::Kind::ArrayPush)
			return &dynamic_cast<MemberAccess const&>(funCall->expression());
	}
	return nullptr;
}

bool SMTEncoder::isPublicGetter(Expression const& _expr) {
	if (!isTrustedExternalCall(&_expr))
		return false;
	auto varDecl = dynamic_cast<VariableDeclaration const*>(
		dynamic_cast<MemberAccess const&>(_expr).annotation().referencedDeclaration
	);
	return varDecl != nullptr;
}

bool SMTEncoder::isTrustedExternalCall(Expression const* _expr) {
	auto memberAccess = dynamic_cast<MemberAccess const*>(_expr);
	if (!memberAccess)
		return false;

	auto identifier = dynamic_cast<Identifier const*>(&memberAccess->expression());
	return identifier &&
		identifier->name() == "this" &&
		identifier->annotation().referencedDeclaration &&
		dynamic_cast<MagicVariableDeclaration const*>(identifier->annotation().referencedDeclaration)
	;
}

string SMTEncoder::extraComment()
{
	string extra;
	if (m_arrayAssignmentHappened)
		extra +=
			"\nNote that array aliasing is not supported,"
			" therefore all mapping information is erased after"
			" a mapping local variable/parameter is assigned.\n"
			"You can re-introduce information using require().";
	return extra;
}

FunctionDefinition const* SMTEncoder::functionCallToDefinition(
	FunctionCall const& _funCall,
	ContractDefinition const* _scopeContract,
	ContractDefinition const* _contextContract
)
{
	if (*_funCall.annotation().kind != FunctionCallKind::FunctionCall)
		return {};

	auto [calledExpr, callOptions] = functionCallExpression(_funCall);

	if (TupleExpression const* fun = dynamic_cast<TupleExpression const*>(calledExpr))
	{
		solAssert(fun->components().size() == 1, "");
		calledExpr = innermostTuple(*calledExpr);
	}

	auto resolveVirtual = [&](auto const* _ref) -> FunctionDefinition const* {
		VirtualLookup lookup = *_ref->annotation().requiredLookup;
		solAssert(_contextContract || lookup == VirtualLookup::Static, "No contract context provided for function lookup resolution!");
		auto funDef = dynamic_cast<FunctionDefinition const*>(_ref->annotation().referencedDeclaration);
		if (!funDef)
			return funDef;
		switch (lookup)
		{
		case VirtualLookup::Virtual:
			return &(funDef->resolveVirtual(*_contextContract));
		case VirtualLookup::Super:
		{
			solAssert(_scopeContract, "");
			auto super = _scopeContract->superContract(*_contextContract);
			solAssert(super, "Super contract not available.");
			return &funDef->resolveVirtual(*_contextContract, super);
		}
		case VirtualLookup::Static:
			return funDef;
		}
		solAssert(false, "");
	};

	if (Identifier const* fun = dynamic_cast<Identifier const*>(calledExpr))
		return resolveVirtual(fun);
	else if (MemberAccess const* fun = dynamic_cast<MemberAccess const*>(calledExpr))
		return resolveVirtual(fun);

	return {};
}

vector<VariableDeclaration const*> SMTEncoder::stateVariablesIncludingInheritedAndPrivate(ContractDefinition const& _contract)
{
	return fold(
		_contract.annotation().linearizedBaseContracts,
		vector<VariableDeclaration const*>{},
		[](auto&& _acc, auto _contract) { return _acc + _contract->stateVariables(); }
	);
}

vector<VariableDeclaration const*> SMTEncoder::stateVariablesIncludingInheritedAndPrivate(FunctionDefinition const& _function)
{
	return stateVariablesIncludingInheritedAndPrivate(dynamic_cast<ContractDefinition const&>(*_function.scope()));
}

vector<VariableDeclaration const*> SMTEncoder::localVariablesIncludingModifiers(FunctionDefinition const& _function, ContractDefinition const* _contract)
{
	return _function.localVariables() + tryCatchVariables(_function) + modifiersVariables(_function, _contract);
}

vector<VariableDeclaration const*> SMTEncoder::tryCatchVariables(FunctionDefinition const& _function)
{
	struct TryCatchVarsVisitor : public ASTConstVisitor
	{
		bool visit(TryCatchClause const& _catchClause) override
		{
			if (_catchClause.parameters())
			{
				auto const& params = _catchClause.parameters()->parameters();
				for (auto param: params)
					vars.push_back(param.get());
			}

			return true;
		}

		vector<VariableDeclaration const*> vars;
	} tryCatchVarsVisitor;
	_function.accept(tryCatchVarsVisitor);
	return tryCatchVarsVisitor.vars;
}

vector<VariableDeclaration const*> SMTEncoder::modifiersVariables(FunctionDefinition const& _function, ContractDefinition const* _contract)
{
	struct BlockVars: ASTConstVisitor
	{
		BlockVars(Block const& _block) { _block.accept(*this); }
		void endVisit(VariableDeclaration const& _var) { vars.push_back(&_var); }
		vector<VariableDeclaration const*> vars;
	};

	vector<VariableDeclaration const*> vars;
	set<ModifierDefinition const*> visited;
	for (auto invok: _function.modifiers())
	{
		if (!invok)
			continue;
		auto const* modifier = resolveModifierInvocation(*invok, _contract);
		if (!modifier || visited.count(modifier))
			continue;

		visited.insert(modifier);
		if (modifier->isImplemented())
		{
			vars += applyMap(modifier->parameters(), [](auto _var) { return _var.get(); });
			vars += BlockVars(modifier->body()).vars;
		}
	}
	return vars;
}

ModifierDefinition const* SMTEncoder::resolveModifierInvocation(ModifierInvocation const& _invocation, ContractDefinition const* _contract)
{
	auto const* modifier = dynamic_cast<ModifierDefinition const*>(_invocation.name().annotation().referencedDeclaration);
	if (modifier)
	{
		VirtualLookup lookup = *_invocation.name().annotation().requiredLookup;
		solAssert(lookup == VirtualLookup::Virtual || lookup == VirtualLookup::Static, "");
		solAssert(_contract || lookup == VirtualLookup::Static, "No contract context provided for modifier lookup resolution!");
		if (lookup == VirtualLookup::Virtual)
			modifier = &modifier->resolveVirtual(*_contract);
	}
	return modifier;
}

set<FunctionDefinition const*, ASTNode::CompareByID> const& SMTEncoder::contractFunctions(ContractDefinition const& _contract)
{
	if (!m_contractFunctions.count(&_contract))
	{
		auto const& functions = _contract.definedFunctions();
		set<FunctionDefinition const*, ASTNode::CompareByID> resolvedFunctions(begin(functions), end(functions));
		for (auto const* base: _contract.annotation().linearizedBaseContracts)
		{
			if (base == &_contract)
				continue;
			for (auto const* baseFunction: base->definedFunctions())
			{
				if (baseFunction->isConstructor()) // We don't want to include constructors of parent contracts
					continue;
				bool overridden = false;
				for (auto const* function: resolvedFunctions)
					if (
						function->name() == baseFunction->name() &&
						function->kind() == baseFunction->kind() &&
						FunctionType(*function).asExternallyCallableFunction(false)->
							hasEqualParameterTypes(*FunctionType(*baseFunction).asExternallyCallableFunction(false))
						)
					{
						overridden = true;
						break;
					}
				if (!overridden)
					resolvedFunctions.insert(baseFunction);
			}
		}
		m_contractFunctions.emplace(&_contract, std::move(resolvedFunctions));
	}
	return m_contractFunctions.at(&_contract);
}

set<FunctionDefinition const*, ASTNode::CompareByID> const& SMTEncoder::contractFunctionsWithoutVirtual(ContractDefinition const& _contract)
{
	if (!m_contractFunctionsWithoutVirtual.count(&_contract))
	{
		auto allFunctions = contractFunctions(_contract);
		for (auto const* base: _contract.annotation().linearizedBaseContracts)
			for (auto const* baseFun: base->definedFunctions())
				allFunctions.insert(baseFun);

		m_contractFunctionsWithoutVirtual.emplace(&_contract, std::move(allFunctions));

	}
	return m_contractFunctionsWithoutVirtual.at(&_contract);
}

SourceUnit const* SMTEncoder::sourceUnitContaining(Scopable const& _scopable)
{
	for (auto const* s = &_scopable; s; s = dynamic_cast<Scopable const*>(s->scope()))
		if (auto const* source = dynamic_cast<SourceUnit const*>(s->scope()))
			return source;
	solAssert(false, "");
}

map<ContractDefinition const*, vector<ASTPointer<frontend::Expression>>> SMTEncoder::baseArguments(ContractDefinition const& _contract)
{
	map<ContractDefinition const*, vector<ASTPointer<Expression>>> baseArgs;

	for (auto contract: _contract.annotation().linearizedBaseContracts)
	{
		/// Collect base contracts and potential constructor arguments.
		for (auto specifier: contract->baseContracts())
		{
			solAssert(specifier, "");
			auto const& base = dynamic_cast<ContractDefinition const&>(*specifier->name().annotation().referencedDeclaration);
			if (auto args = specifier->arguments())
				baseArgs[&base] = *args;
		}
		/// Collect base constructor arguments given as constructor modifiers.
		if (auto constructor = contract->constructor())
			for (auto mod: constructor->modifiers())
			{
				auto decl = mod->name().annotation().referencedDeclaration;
				if (auto base = dynamic_cast<ContractDefinition const*>(decl))
				{
					solAssert(!baseArgs.count(base), "");
					if (auto args = mod->arguments())
						baseArgs[base] = *args;
				}
			}
	}

	return baseArgs;
}

RationalNumberType const* SMTEncoder::isConstant(Expression const& _expr)
{
	if (auto type = dynamic_cast<RationalNumberType const*>(_expr.annotation().type))
		return type;

	// _expr may not be constant evaluable.
	// In that case we ignore any warnings emitted by the constant evaluator,
	// as it will return nullptr in case of failure.
	ErrorList l;
	ErrorReporter e(l);
	if (auto t = ConstantEvaluator::evaluate(e, _expr))
		return TypeProvider::rationalNumber(t->value);

	return nullptr;
}

set<FunctionCall const*> SMTEncoder::collectABICalls(ASTNode const* _node)
{
	struct ABIFunctions: public ASTConstVisitor
	{
		ABIFunctions(ASTNode const* _node) { _node->accept(*this); }
		void endVisit(FunctionCall const& _funCall)
		{
			if (*_funCall.annotation().kind == FunctionCallKind::FunctionCall)
				switch (dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type).kind())
				{
				case FunctionType::Kind::ABIEncode:
				case FunctionType::Kind::ABIEncodePacked:
				case FunctionType::Kind::ABIEncodeWithSelector:
				case FunctionType::Kind::ABIEncodeCall:
				case FunctionType::Kind::ABIEncodeWithSignature:
				case FunctionType::Kind::ABIDecode:
					abiCalls.insert(&_funCall);
					break;
				default: {}
				}
		}

		set<FunctionCall const*> abiCalls;
	};

	return ABIFunctions(_node).abiCalls;
}

set<SourceUnit const*, ASTNode::CompareByID> SMTEncoder::sourceDependencies(SourceUnit const& _source)
{
	set<SourceUnit const*, ASTNode::CompareByID> sources;
	sources.insert(&_source);
	for (auto const& source: _source.referencedSourceUnits(true))
		sources.insert(source);
	return sources;
}

void SMTEncoder::createReturnedExpressions(FunctionCall const& _funCall, ContractDefinition const* _contextContract)
{
	auto funDef = functionCallToDefinition(_funCall, currentScopeContract(), _contextContract);
	if (!funDef)
		return;

	auto const& returnParams = funDef->returnParameters();
	for (auto param: returnParams)
		createVariable(*param);
	auto returnValues = applyMap(returnParams, [this](auto const& param) -> optional<smtutil::Expression> {
		solAssert(param && m_context.knownVariable(*param), "");
		return currentValue(*param);
	});
	defineExpr(_funCall, returnValues);
}

vector<smtutil::Expression> SMTEncoder::symbolicArguments(FunctionCall const& _funCall, ContractDefinition const* _contextContract)
{
	auto funDef = functionCallToDefinition(_funCall, currentScopeContract(), _contextContract);
	solAssert(funDef, "");

	vector<smtutil::Expression> args;
	Expression const* calledExpr = &_funCall.expression();
	auto funType = dynamic_cast<FunctionType const*>(calledExpr->annotation().type);
	solAssert(funType, "");

	vector<ASTPointer<Expression const>> arguments = _funCall.sortedArguments();
	auto functionParams = funDef->parameters();
	unsigned firstParam = 0;
	if (funType->boundToType())
	{
		calledExpr = innermostTuple(*calledExpr);
		auto const& boundFunction = dynamic_cast<MemberAccess const*>(calledExpr);
		solAssert(boundFunction, "");
		args.push_back(expr(boundFunction->expression(), functionParams.front()->type()));
		firstParam = 1;
	}

	solAssert((arguments.size() + firstParam) == functionParams.size(), "");
	for (unsigned i = 0; i < arguments.size(); ++i)
		args.push_back(expr(*arguments.at(i), functionParams.at(i + firstParam)->type()));

	return args;
}

smtutil::Expression SMTEncoder::constantExpr(Expression const& _expr, VariableDeclaration const& _var)
{
	if (RationalNumberType const* rationalType = isConstant(_expr))
	{
		if (rationalType->isNegative())
			return smtutil::Expression(u2s(rationalType->literalValue(nullptr)));
		else
			return smtutil::Expression(rationalType->literalValue(nullptr));
	}
	else
	{
		solAssert(_var.value(), "");
		_var.value()->accept(*this);
		return expr(*_var.value(), _expr.annotation().type);
	}
	solAssert(false, "");
}

void SMTEncoder::collectFreeFunctions(set<SourceUnit const*, ASTNode::CompareByID> const& _sources)
{
	for (auto source: _sources)
		for (auto node: source->nodes())
			if (auto function = dynamic_cast<FunctionDefinition const*>(node.get()))
				m_freeFunctions.insert(function);
			else if (
				auto contract = dynamic_cast<ContractDefinition const*>(node.get());
				contract && contract->isLibrary()
			)
			{
				for (auto function: contract->definedFunctions())
					if (!function->isPublic())
						m_freeFunctions.insert(function);
			}
}

void SMTEncoder::createFreeConstants(set<SourceUnit const*, ASTNode::CompareByID> const& _sources)
{
	for (auto source: _sources)
		for (auto node: source->nodes())
			if (auto var = dynamic_cast<VariableDeclaration const*>(node.get()))
				createVariable(*var);
			else if (
				auto contract = dynamic_cast<ContractDefinition const*>(node.get());
				contract && contract->isLibrary()
			)
				for (auto var: contract->stateVariables())
				{
					solAssert(var->isConstant(), "");
					createVariable(*var);
				}
}

smt::SymbolicState& SMTEncoder::state()
{
	return m_context.state();
}
