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

/// Mutate expression into an s/m/calldataload
static YulProtoMutator addLoadZero(
	Expression::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto expr = static_cast<Expression*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: expression mutated to load op" << std::endl;
#endif
			switch (expr->expr_oneof_case())
			{
			case Expression::kVarref:
				expr->clear_varref();
				break;
			case Expression::kCons:
				expr->clear_cons();
				break;
			case Expression::kBinop:
				expr->clear_binop();
				break;
			case Expression::kUnop:
				expr->clear_unop();
				break;
			case Expression::kTop:
				expr->clear_top();
				break;
			case Expression::kNop:
				expr->clear_nop();
				break;
			case Expression::kFuncExpr:
				expr->clear_func_expr();
				break;
			case Expression::kLowcall:
				expr->clear_lowcall();
				break;
			case Expression::kCreate:
				expr->clear_create();
				break;
			case Expression::kUnopdata:
				expr->clear_unopdata();
				break;
			case Expression::EXPR_ONEOF_NOT_SET:
				break;
			}
			expr->set_allocated_unop(YulProtoMutator::loadExpression(_seed));
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove unary operation containing a load from memory/storage/calldata
static YulProtoMutator removeLoad(
	UnaryOp::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto unaryOp = static_cast<UnaryOp*>(_message);
		auto operation = unaryOp->op();
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (operation == UnaryOp::MLOAD ||
				operation == UnaryOp::SLOAD ||
				operation == UnaryOp::CALLDATALOAD
			)
			{
#ifdef DEBUG
				std::cout << "----------------------------------" << std::endl;
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Remove mload" << std::endl;
#endif
				unaryOp->clear_op();
				unaryOp->clear_operand();
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "----------------------------------" << std::endl;
#endif
			}
		}
	}
);

/// Add m/sstore(0, variable)
static YulProtoMutator addStoreToZero(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto block = static_cast<Block*>(_message);
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: store added" << std::endl;
#endif
			auto storeStmt = new StoreFunc();
			storeStmt->set_st(YulProtoMutator::EnumTypeConverter<StoreFunc_Storage>{}.enumFromSeed(_seed));
			// One in two times, we force a store(0, varref). In the other instance,
			// we leave loc and val unset.
			if (_seed % 2)
			{
				storeStmt->set_allocated_loc(YulProtoMutator::litExpression(0));
				storeStmt->set_allocated_val(YulProtoMutator::refExpression(_seed));
			}
			auto stmt = block->add_statements();
			stmt->set_allocated_storage_func(storeStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove m/sstore(0, ref)
static YulProtoMutator removeStore(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto block = static_cast<Block*>(_message);
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove store" << std::endl;
#endif
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_storage_func())
				{
					stmt.clear_storage_func();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add switch" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto switchStmt = new SwitchStmt();
			switchStmt->add_case_stmt();
			Expression *switchExpr = new Expression();
			switchExpr->set_allocated_varref(YulProtoMutator::varRef(_seed));
			switchStmt->set_allocated_switch_expr(switchExpr);
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
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
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
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

/// Add function definition
static YulProtoMutator addFuncDef(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add function def" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto funcDef = new FunctionDef();
			funcDef->set_num_input_params(_seed);
			funcDef->set_num_output_params(_seed + block->ByteSizeLong());
			// Copy block into function body
			auto funcBlock = new Block();
			funcBlock->CopyFrom(*block);
			funcDef->set_allocated_block(funcBlock);
			stmt->set_allocated_funcdef(funcDef);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove function definition
static YulProtoMutator removeFuncDef(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove function def" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_funcdef())
				{
					stmt.clear_funcdef();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add bounded for stmt
static YulProtoMutator addBoundedFor(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add bounded for stmt" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto forStmt = new BoundedForStmt();
			stmt->set_allocated_boundedforstmt(forStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove bounded for stmt
static YulProtoMutator removeBoundedFor(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove bounded for stmt" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_boundedforstmt())
				{
					stmt.clear_boundedforstmt();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add generic for stmt
static YulProtoMutator addGenericFor(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add generic for stmt" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto forStmt = new ForStmt();
			stmt->set_allocated_forstmt(forStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove generic for stmt
static YulProtoMutator removeGenericFor(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove generic for stmt" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_forstmt())
				{
					stmt.clear_forstmt();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add revert stmt
static YulProtoMutator addTerminatingStmt(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add terminating stmt" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto termStmt = new TerminatingStmt();
			auto revertStmt = new RetRevStmt();
			revertStmt->set_stmt(RetRevStmt::REVERT);
			revertStmt->set_allocated_pos(YulProtoMutator::litExpression(0));
			revertStmt->set_allocated_size(YulProtoMutator::litExpression(0));
			termStmt->set_allocated_ret_rev(revertStmt);
			stmt->set_allocated_terminatestmt(termStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove revert statement
static YulProtoMutator removeRevertStmt(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove revert stmt" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_terminatestmt() &&
					stmt.terminatestmt().has_ret_rev() &&
					stmt.terminatestmt().ret_rev().stmt() == RetRevStmt::REVERT
				)
				{
					stmt.clear_terminatestmt();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Mutate nullary op
static YulProtoMutator mutateNullaryOp(
	NullaryOp::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Mutate Nullary op" << std::endl;
#endif
			auto nullOpExpr = static_cast<NullaryOp*>(_message);
			nullOpExpr->clear_op();
			nullOpExpr->set_op(
				YulProtoMutator::EnumTypeConverter<NullaryOp_NOp>{}.enumFromSeed(_seed)
			);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Mutate binary op
static YulProtoMutator mutateBinaryOp(
	BinaryOp::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Mutate Binary op" << std::endl;
#endif
			auto binOpExpr = static_cast<BinaryOp*>(_message);
			binOpExpr->clear_op();
			binOpExpr->set_op(
				YulProtoMutator::EnumTypeConverter<BinaryOp_BOp>{}.enumFromSeed(_seed)
			);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Mutate unary op
static YulProtoMutator mutateUnaryOp(
	UnaryOp::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Mutate Unary op" << std::endl;
#endif
			auto unaryOpExpr = static_cast<UnaryOp*>(_message);
			unaryOpExpr->clear_op();
			unaryOpExpr->set_op(
				YulProtoMutator::EnumTypeConverter<UnaryOp_UOp>{}.enumFromSeed(_seed)
			);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);


Literal* YulProtoMutator::intLiteral(unsigned _value)
{
	auto lit = new Literal();
	lit->set_intval(_value);
	return lit;
}

Expression* YulProtoMutator::litExpression(unsigned _value)
{
	auto lit = intLiteral(_value);
	auto expr = new Expression();
	expr->set_allocated_cons(lit);
	return expr;
}

VarRef* YulProtoMutator::varRef(unsigned _seed)
{
	auto varref = new VarRef();
	varref->set_varnum(_seed);
	return varref;
}

Expression* YulProtoMutator::refExpression(unsigned _seed)
{
	auto ref = varRef(_seed);
	auto refExpr = new Expression();
	refExpr->set_allocated_varref(ref);
	return refExpr;
}

void YulProtoMutator::configureCall(FunctionCall *_call, unsigned int _seed)
{
	auto type = EnumTypeConverter<FunctionCall_Returns>{}.enumFromSeed(_seed);
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

template <typename T>
T YulProtoMutator::EnumTypeConverter<T>::validEnum(unsigned _seed)
{
	auto ret = static_cast<T>(_seed % (enumMax() - enumMin() + 1) + enumMin());
	if constexpr (std::is_same_v<std::decay_t<T>, FunctionCall_Returns>)
		yulAssert(FunctionCall_Returns_IsValid(ret), "Yul proto mutator: Invalid enum");
	else if constexpr (std::is_same_v<std::decay_t<T>, StoreFunc_Storage>)
		yulAssert(StoreFunc_Storage_IsValid(ret), "Yul proto mutator: Invalid enum");
	else if constexpr (std::is_same_v<std::decay_t<T>, NullaryOp_NOp>)
		yulAssert(NullaryOp_NOp_IsValid(ret), "Yul proto mutator: Invalid enum");
	else if constexpr (std::is_same_v<std::decay_t<T>, BinaryOp_BOp>)
		yulAssert(BinaryOp_BOp_IsValid(ret), "Yul proto mutator: Invalid enum");
	else if constexpr (std::is_same_v<std::decay_t<T>, UnaryOp_UOp>)
		yulAssert(UnaryOp_UOp_IsValid(ret), "Yul proto mutator: Invalid enum");
	else
		static_assert(AlwaysFalse<T>::value, "Yul proto mutator: non-exhaustive visitor.");
	return ret;
}

template <typename T>
int YulProtoMutator::EnumTypeConverter<T>::enumMax()
{
	if constexpr (std::is_same_v<std::decay_t<T>, FunctionCall_Returns>)
		return FunctionCall_Returns_Returns_MAX;
	else if constexpr (std::is_same_v<std::decay_t<T>, StoreFunc_Storage>)
		return StoreFunc_Storage_Storage_MAX;
	else if constexpr (std::is_same_v<std::decay_t<T>, NullaryOp_NOp>)
		return NullaryOp_NOp_NOp_MAX;
	else if constexpr (std::is_same_v<std::decay_t<T>, BinaryOp_BOp>)
		return BinaryOp_BOp_BOp_MAX;
	else if constexpr (std::is_same_v<std::decay_t<T>, UnaryOp_UOp>)
		return UnaryOp_UOp_UOp_MAX;
	else
		static_assert(AlwaysFalse<T>::value, "Yul proto mutator: non-exhaustive visitor.");
}

template <typename T>
int YulProtoMutator::EnumTypeConverter<T>::enumMin()
{
	if constexpr (std::is_same_v<std::decay_t<T>, FunctionCall_Returns>)
		return FunctionCall_Returns_Returns_MIN;
	else if constexpr (std::is_same_v<std::decay_t<T>, StoreFunc_Storage>)
		return StoreFunc_Storage_Storage_MIN;
	else if constexpr (std::is_same_v<std::decay_t<T>, NullaryOp_NOp>)
		return NullaryOp_NOp_NOp_MIN;
	else if constexpr (std::is_same_v<std::decay_t<T>, BinaryOp_BOp>)
		return BinaryOp_BOp_BOp_MIN;
	else if constexpr (std::is_same_v<std::decay_t<T>, UnaryOp_UOp>)
		return UnaryOp_UOp_UOp_MIN;
	else
		static_assert(AlwaysFalse<T>::value, "Yul proto mutator: non-exhaustive visitor.");
}

UnaryOp* YulProtoMutator::loadExpression(unsigned _seed)
{
	auto unop = new UnaryOp();
	unop->set_allocated_operand(litExpression(0));
	switch (_seed % 3)
	{
	case 0:
		unop->set_op(UnaryOp::MLOAD);
		break;
	case 1:
		unop->set_op(UnaryOp::SLOAD);
		break;
	case 2:
		unop->set_op(UnaryOp::CALLDATALOAD);
		break;
	}
	return unop;
}