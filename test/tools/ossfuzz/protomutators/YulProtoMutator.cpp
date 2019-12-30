#include <test/tools/ossfuzz/protomutators/YulProtoMutator.h>

#include <src/text_format.h>
#include <libyul/Exceptions.h>

#define DEBUG

using namespace yul::test::yul_fuzzer;

/// Invert condition of an if statement
static YulProtoMutator invertIfCondition(
	IfStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto ifStmt = static_cast<IfStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (ifStmt->has_cond())
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: If condition inverted" << std::endl;
#endif
				auto notOp = new UnaryOp();
				notOp->set_op(UnaryOp::NOT);
				auto oldCond = ifStmt->release_cond();
				notOp->set_allocated_operand(oldCond);
				auto ifCond = new Expression();
				ifCond->set_allocated_unop(notOp);
				ifStmt->set_allocated_cond(ifCond);
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Remove inverted condition in if statement
static YulProtoMutator removeInvertedIfCondition(
	IfStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto ifStmt = static_cast<IfStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (ifStmt->has_cond() &&
				ifStmt->cond().has_unop() &&
				ifStmt->cond().unop().has_op() &&
				ifStmt->cond().unop().op() == UnaryOp::NOT
			)
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Remove If condition inverted" << std::endl;
#endif
				auto oldCondition = ifStmt->release_cond();
				auto unop = oldCondition->release_unop();
				auto conditionWithoutNot = unop->release_operand();
				ifStmt->set_allocated_cond(conditionWithoutNot);
				delete(oldCondition);
				delete(unop);
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Add break statement in body of a for-loop statement
static YulProtoMutator addBreak(
	ForStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (forStmt->has_for_body())
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Break added" << std::endl;
#endif
				auto breakStmt = new BreakStmt();
				auto statement = forStmt->mutable_for_body()->add_statements();
				statement->set_allocated_breakstmt(breakStmt);
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Remove break statement in body of a for-loop statement
static YulProtoMutator removeBreak(
	ForStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (forStmt->has_for_body())
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Remove Break" << std::endl;
#endif
				for (auto &stmt: *forStmt->mutable_for_body()->mutable_statements())
					if (stmt.has_breakstmt())
					{
						stmt.clear_breakstmt();
						break;
					}
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Add continue statement in body of a for-loop statement
static YulProtoMutator addContinue(
	ForStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (forStmt->has_for_body())
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Continue added" << std::endl;
#endif
				auto contStmt = new ContinueStmt();
				auto statement = forStmt->mutable_for_body()->add_statements();
				statement->set_allocated_contstmt(contStmt);
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Remove continue statement in body of a for-loop statement
static YulProtoMutator removeContinue(
	ForStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (forStmt->has_for_body())
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Remove Continue" << std::endl;
#endif
				for (auto &stmt: *forStmt->mutable_for_body()->mutable_statements())
					if (stmt.has_contstmt())
					{
						stmt.clear_contstmt();
						break;
					}
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Add declaration statement referencing mload(0)
static YulProtoMutator addMloadZero(
	VarDecl::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto varDeclStmt = static_cast<VarDecl*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (varDeclStmt->has_expr())
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: mload added" << std::endl;
#endif
				varDeclStmt->clear_expr();
				auto zeroLit = YulProtoMutator::intLiteral(0);
				auto consExpr = new Expression();
				consExpr->set_allocated_cons(zeroLit);
				auto mloadOp = new UnaryOp();
				mloadOp->set_op(UnaryOp::MLOAD);
				mloadOp->set_allocated_operand(consExpr);
				auto mloadExpr = new Expression();
				mloadExpr->set_allocated_unop(mloadOp);
				varDeclStmt->set_allocated_expr(mloadExpr);
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Invert condition of a for statement
static YulProtoMutator invertForCondition(
	ForStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (forStmt->has_for_cond())
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: For condition inverted" << std::endl;
#endif
				auto notOp = new UnaryOp();
				notOp->set_op(UnaryOp::NOT);
				auto oldCond = forStmt->release_for_cond();
				notOp->set_allocated_operand(oldCond);
				auto forCond = new Expression();
				forCond->set_allocated_unop(notOp);
				forStmt->set_allocated_for_cond(forCond);
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Remove inverted condition of a for statement
static YulProtoMutator removeInvertedForCondition(
	ForStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (forStmt->has_for_cond() &&
				forStmt->for_cond().has_unop() &&
				forStmt->for_cond().unop().has_op() &&
				forStmt->for_cond().unop().op() == UnaryOp::NOT
			)
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Remove For condition inverted" << std::endl;
#endif
				auto oldCondition = forStmt->release_for_cond();
				auto unop = oldCondition->release_unop();
				auto newCondition = unop->release_operand();
				forStmt->set_allocated_for_cond(newCondition);
				delete oldCondition;
				delete unop;
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Make for loop condition a function call that returns a single value
static YulProtoMutator funcCallForCondition(
	ForStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (forStmt->has_for_cond())
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Function call in for condition" << std::endl;
#endif
				forStmt->release_for_cond();
				auto functionCall = new FunctionCall();
				functionCall->set_ret(FunctionCall::SINGLE);
				functionCall->set_func_index(_seed);
				auto forCondExpr = new Expression();
				forCondExpr->set_allocated_func_expr(functionCall);
				forStmt->set_allocated_for_cond(forCondExpr);
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Define an identity function y = x
static YulProtoMutator identityFunction(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Identity function" << std::endl;
#endif
			auto blockStmt = static_cast<Block*>(_message);
			auto functionDef = new FunctionDef();
			functionDef->set_num_input_params(1);
			functionDef->set_num_output_params(1);
			auto functionBlock = new Block();
			auto assignmentStatement = new AssignmentStatement();
			auto varRef = YulProtoMutator::varRef(_seed);
			assignmentStatement->set_allocated_ref_id(varRef);
			auto rhs = new Expression();
			auto rhsRef = YulProtoMutator::varRef(_seed + blockStmt->ByteSizeLong());
			rhs->set_allocated_varref(rhsRef);
			assignmentStatement->set_allocated_expr(rhs);
			auto stmt = functionBlock->add_statements();
			stmt->set_allocated_assignment(assignmentStatement);
			functionDef->set_allocated_block(functionBlock);
			auto funcdefStmt = blockStmt->add_statements();
			funcdefStmt->set_allocated_funcdef(functionDef);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add leave statement to an existing function.
static YulProtoMutator addLeave(
	FunctionDef::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_lowIP == 0) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Leave in function" << std::endl;
#endif
			auto funcDef = static_cast<FunctionDef*>(_message);
			auto newStmt = funcDef->mutable_block()->add_statements();
			auto leaveStmt = new LeaveStmt();
			newStmt->set_allocated_leave(leaveStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove leave statement from an existing function.
static YulProtoMutator removeLeave(
	FunctionDef::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_lowIP == 1) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove Leave in function" << std::endl;
#endif
			auto funcDef = static_cast<FunctionDef*>(_message);
			for (auto &stmt: *funcDef->mutable_block()->mutable_statements())
				if (stmt.has_leave())
				{
					stmt.clear_leave();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add assignment to block
static YulProtoMutator addAssignment(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add assignment" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto assignmentStatement = new AssignmentStatement();
			auto varRef = YulProtoMutator::varRef(_seed);
			assignmentStatement->set_allocated_ref_id(varRef);
			auto rhs = YulProtoMutator::varRef(_seed + block->ByteSizeLong());
			auto rhsExpr = new Expression();
			rhsExpr->set_allocated_varref(rhs);
			assignmentStatement->set_allocated_expr(rhsExpr);
			auto newStmt = block->add_statements();
			newStmt->set_allocated_assignment(assignmentStatement);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove assignment from block
static YulProtoMutator removeAssignment(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 1) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove assignment" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_assignment())
				{
					stmt.clear_assignment();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add if statement
static YulProtoMutator addIf(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add if" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto ifStmt = new IfStmt();
			stmt->set_allocated_ifstmt(ifStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove if statement
static YulProtoMutator removeIf(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 1) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove if" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_ifstmt())
				{
					stmt.clear_ifstmt();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add switch statement
static YulProtoMutator addSwitch(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add switch" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto switchStmt = new SwitchStmt();
			stmt->set_allocated_switchstmt(switchStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove switch statement
static YulProtoMutator removeSwitch(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 1) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove switch" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_switchstmt())
				{
					stmt.clear_switchstmt();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add function call
static YulProtoMutator addCall(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add function call" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto call = new FunctionCall();
			YulProtoMutator::configureCall(call, _seed);
			stmt->set_allocated_functioncall(call);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove function call
static YulProtoMutator removeCall(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 1) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove function call" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_functioncall())
				{
					stmt.clear_functioncall();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add variable declaration
static YulProtoMutator addVarDecl(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add variable decl" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto varDecl = new VarDecl();
			stmt->set_allocated_decl(varDecl);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove variable declaration
static YulProtoMutator removeVarDecl(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 1) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove var decl" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_decl())
				{
					stmt.clear_decl();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

Literal* YulProtoMutator::intLiteral(unsigned _value)
{
	Literal *lit = new Literal();
	lit->set_intval(_value);
	return lit;
}

VarRef* YulProtoMutator::varRef(unsigned _seed)
{
	VarRef *varref = new VarRef();
	varref->set_varnum(_seed);
	return varref;
}

FunctionCall_Returns YulProtoMutator::callType(unsigned _seed)
{
	switch (_seed % 4)
	{
	case 0:
		return FunctionCall_Returns_ZERO;
	case 1:
		return FunctionCall_Returns_SINGLE;
	case 2:
		return FunctionCall_Returns_MULTIASSIGN;
	case 3:
		return FunctionCall_Returns_MULTIDECL;
	default:
		yulAssert(false, "");
	}
}

void YulProtoMutator::configureCall(FunctionCall *_call, unsigned int _seed)
{
	auto type = callType(_seed);
	_call->set_ret(type);
	_call->set_func_index(_seed);
	// Configuration rules:
	// - In/Out parameters do not need to be configured for zero I/O call
	// - Single in and zero out parameter needs to be configured for single I/O call
	// - Four in and no out parameters need to be configured for multidecl call
	// - Four in and out parameters need to be configured for multiassign call
	switch (type)
	{
	case FunctionCall_Returns_MULTIASSIGN:
	{
		auto outRef4 = YulProtoMutator::varRef(_seed * 8);
		_call->set_allocated_out_param4(outRef4);

		auto outRef3 = YulProtoMutator::varRef(_seed * 7);
		_call->set_allocated_out_param3(outRef3);

		auto outRef2 = YulProtoMutator::varRef(_seed * 6);
		_call->set_allocated_out_param2(outRef2);

		auto outRef1 = YulProtoMutator::varRef(_seed * 5);
		_call->set_allocated_out_param1(outRef1);
	}
	BOOST_FALLTHROUGH;
	case FunctionCall_Returns_MULTIDECL:
	{
		auto inArg4 = new Expression();
		auto inRef4 = YulProtoMutator::varRef(_seed * 4);
		inArg4->set_allocated_varref(inRef4);
		_call->set_allocated_in_param4(inArg4);

		auto inArg3 = new Expression();
		auto inRef3 = YulProtoMutator::varRef(_seed * 3);
		inArg3->set_allocated_varref(inRef3);
		_call->set_allocated_in_param3(inArg3);

		auto inArg2 = new Expression();
		auto inRef2 = YulProtoMutator::varRef(_seed * 2);
		inArg2->set_allocated_varref(inRef2);
		_call->set_allocated_in_param2(inArg2);
	}
	BOOST_FALLTHROUGH;
	case FunctionCall_Returns_SINGLE:
	{
		auto inArg1 = new Expression();
		auto inRef1 = YulProtoMutator::varRef(_seed);
		inArg1->set_allocated_varref(inRef1);
		_call->set_allocated_in_param1(inArg1);
	}
	BOOST_FALLTHROUGH;
	case FunctionCall_Returns_ZERO:
		break;
	}
}