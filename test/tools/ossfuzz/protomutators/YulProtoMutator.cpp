#include <test/tools/ossfuzz/protomutators/YulProtoMutator.h>

#include <src/text_format.h>
#include <libyul/Exceptions.h>

#define DEBUG

using namespace solidity::yul::test::yul_fuzzer;

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

/// Append break statement to a statement block
static YulProtoMutator addBreak(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto block = static_cast<Block*>(_message);
		if (_seed % 1/*YulProtoMutator::s_highIP*/ == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Break added" << std::endl;
#endif
			auto breakStmt = new BreakStmt();
			auto statement = block->add_statements();
			statement->set_allocated_breakstmt(breakStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
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
						delete stmt.release_breakstmt();
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
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		auto block = static_cast<Block*>(_message);
		if (_seed % 1/*YulProtoMutator::s_highIP*/ == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Continue added" << std::endl;
#endif
			auto contStmt = new ContinueStmt();
			auto statement = block->add_statements();
			statement->set_allocated_contstmt(contStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
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
						delete stmt.release_contstmt();
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
		if (_seed % YulProtoMutator::s_highIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: expression mutated to load op" << std::endl;
#endif
			YulProtoMutator::clearExpr(expr);
			expr = YulProtoMutator::loadExpression(_seed/23);
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
				delete unaryOp->release_operand();
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
		if (_seed % 1/*YulProtoMutator::s_normalizedBlockIP*/ == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: store added" << std::endl;
#endif
			auto storeStmt = new StoreFunc();
			storeStmt->set_st(YulProtoMutator::EnumTypeConverter<StoreFunc_Storage>{}.enumFromSeed(_seed/37));
			storeStmt->set_allocated_loc(YulProtoMutator::litExpression(0));
			storeStmt->set_allocated_val(YulProtoMutator::refExpression(_seed/11));
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
					delete stmt.release_storage_func();
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
				functionCall->set_func_index(_seed/41);
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
			auto varRef = YulProtoMutator::varRef(_seed/47);
			assignmentStatement->set_allocated_ref_id(varRef);
			auto rhs = new Expression();
			auto rhsRef = YulProtoMutator::varRef(_seed/79);
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

/// Add leave statement to a statement block
static YulProtoMutator addLeave(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % 1/*YulProtoMutator::s_highIP*/ == 0) {
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add leave" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto newStmt = block->add_statements();
			auto leaveStmt = new LeaveStmt();
			newStmt->set_allocated_leave(leaveStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove leave statement from a statement block
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
					delete stmt.release_leave();
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
			auto varRef = YulProtoMutator::varRef(_seed/11);
			assignmentStatement->set_allocated_ref_id(varRef);
			auto rhs = YulProtoMutator::varRef(_seed/17);
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
					delete stmt.release_assignment();
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

/// Add constant assignment
static YulProtoMutator addConstantAssignment(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add constant assignment" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto assignmentStatement = new AssignmentStatement();
			assignmentStatement->set_allocated_ref_id(
				YulProtoMutator::varRef(_seed/11)
			);
			assignmentStatement->set_allocated_expr(
				YulProtoMutator::litExpression(_seed/59)
			);
			auto newStmt = block->add_statements();
			newStmt->set_allocated_assignment(assignmentStatement);
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
		if (_seed % 1/*YulProtoMutator::s_normalizedBlockIP*/ == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add if" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto ifStmt = new IfStmt();
			ifStmt->set_allocated_cond(YulProtoMutator::refExpression(_seed/11));
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
		if (_seed % YulProtoMutator::s_lowIP == 1)
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
					delete stmt.release_ifstmt();
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
		if (_seed % 1/*YulProtoMutator::s_normalizedBlockIP*/ == 0)
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
			switchExpr->set_allocated_varref(YulProtoMutator::varRef(_seed/11));
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
		if (_seed % YulProtoMutator::s_lowIP == 1)
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
					delete stmt.release_switchstmt();
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
					delete stmt.release_functioncall();
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
		if (_seed % 1/*YulProtoMutator::s_normalizedBlockIP*/ == 0)
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
			// Hoist var decl to beginning of block
			if (block->statements_size() > 1)
				block->mutable_statements(0)->Swap(block->mutable_statements(block->statements_size() - 1));
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add multivar decl
/// Add variable declaration
static YulProtoMutator addMultiVarDecl(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % 1/*YulProtoMutator::s_normalizedBlockIP*/ == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add multi variable decl" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			auto multiVarDecl = new MultiVarDecl();
			multiVarDecl->set_num_vars(_seed/17);
			stmt->set_allocated_multidecl(multiVarDecl);
			// Hoist multi var decl to beginning of block
			if (block->statements_size() > 1)
				block->mutable_statements(0)->Swap(block->mutable_statements(block->statements_size() - 1));
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
					delete stmt.release_decl();
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
			auto funcDef = new FunctionDef();
			funcDef->set_num_input_params(_seed/11);
			funcDef->set_num_output_params(_seed/17);
			auto stmt = block->add_statements();
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
					delete stmt.release_funcdef();
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
		if (_seed % YulProtoMutator::s_lowIP == 1)
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
					delete stmt.release_boundedforstmt();
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
		if (_seed % YulProtoMutator::s_lowIP == 1)
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
					delete stmt.release_forstmt();
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
static YulProtoMutator addRevertStmt(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % 1/*YulProtoMutator::s_normalizedBlockIP*/ == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add revert stmt" << std::endl;
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
		if (_seed % YulProtoMutator::s_lowIP == 1)
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
					delete stmt.release_terminatestmt();
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
				YulProtoMutator::EnumTypeConverter<NullaryOp_NOp>{}.enumFromSeed(_seed/11)
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
				YulProtoMutator::EnumTypeConverter<BinaryOp_BOp>{}.enumFromSeed(_seed/11)
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
				YulProtoMutator::EnumTypeConverter<UnaryOp_UOp>{}.enumFromSeed(_seed/11)
			);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add pop(call())
static YulProtoMutator addPopCall(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add pop(call) stmt" << std::endl;
#endif
			auto call = new LowLevelCall();
			call->set_callty(
				YulProtoMutator::EnumTypeConverter<LowLevelCall_Type>{}.enumFromSeed(_seed/13)
			);
			auto popExpr = new Expression();
			popExpr->set_allocated_lowcall(call);
			auto popStmt = new PopStmt();
			popStmt->set_allocated_expr(popExpr);
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			stmt->set_allocated_pop(popStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove pop
static YulProtoMutator removePopStmt(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 1)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove pop stmt" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_pop())
				{
					delete stmt.release_pop();
					stmt.clear_pop();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add pop(create)
static YulProtoMutator addPopCreate(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add pop(create) stmt" << std::endl;
#endif
			auto create = new Create();
			create->set_createty(
				YulProtoMutator::EnumTypeConverter<Create_Type>{}.enumFromSeed(_seed/17)
			);
			auto popExpr = new Expression();
			popExpr->set_allocated_create(create);
			auto popStmt = new PopStmt();
			popStmt->set_allocated_expr(popExpr);
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			stmt->set_allocated_pop(popStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add pop(f()) where f() -> r is a user-defined function
/// Assumes that f() already exists, if it doesn't this
/// turns into pop(constant)
static YulProtoMutator addPopUserFunction(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_normalizedBlockIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add pop(f()) stmt" << std::endl;
#endif
			auto functioncall = new FunctionCall();
			functioncall->set_ret(FunctionCall::SINGLE);
			YulProtoMutator::configureCallArgs(FunctionCall::SINGLE, functioncall, _seed);
			auto funcExpr = new Expression();
			funcExpr->set_allocated_func_expr(functioncall);
			auto popStmt = new PopStmt();
			popStmt->set_allocated_expr(funcExpr);
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			stmt->set_allocated_pop(popStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add function call in another function's body
static YulProtoMutator addFuncCallInFuncBody(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add function call to statement block" << std::endl;
#endif
			auto functioncall = new FunctionCall();
			YulProtoMutator::configureCall(functioncall, _seed);
			auto block = static_cast<Block*>(_message);
			auto stmt = block->add_statements();
			stmt->set_allocated_functioncall(functioncall);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Remove function call from another function's body
static YulProtoMutator removeFuncCallInFuncBody(
	FunctionDef::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove function call in func body" << std::endl;
#endif
			for (auto &stmt: *static_cast<FunctionDef*>(_message)->mutable_block()->mutable_statements())
				if (stmt.has_functioncall())
				{
					delete stmt.release_functioncall();
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

/// Add dataoffset/datasize
static YulProtoMutator addDataOffset(
	Expression::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Mutate expression to dataoffset()" << std::endl;
#endif
			Expression *expr = static_cast<Expression*>(_message);
			YulProtoMutator::clearExpr(expr);
			auto unopdata = new UnaryOpData();
			auto objId = new ObjectId();
			objId->set_id(_seed/23);
			unopdata->set_allocated_identifier(objId);
			unopdata->set_op(
				YulProtoMutator::EnumTypeConverter<UnaryOpData_UOpData>{}.enumFromSeed(_seed/29)
			);
			expr->set_allocated_unopdata(unopdata);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add variable reference inside for-loop body
static YulProtoMutator addVarRefInForBody(
	BoundedForStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add var ref inside bounded for loop body" << std::endl;
#endif
			auto forStmt = static_cast<BoundedForStmt*>(_message);
			auto popStmt = new PopStmt();
			popStmt->set_allocated_expr(YulProtoMutator::refExpression(_seed/31));
			auto newStmt = forStmt->mutable_for_body()->add_statements();
			newStmt->set_allocated_pop(popStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Mutate expression to a function call
static YulProtoMutator mutateExprToFuncCall(
	Expression::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_highIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Mutate expression to function call" << std::endl;
#endif
			auto expr = static_cast<Expression*>(_message);
			YulProtoMutator::clearExpr(expr);
			auto functionCall = new FunctionCall();
			functionCall->set_ret(FunctionCall::SINGLE);
			functionCall->set_func_index(_seed/17);
			expr->set_allocated_func_expr(functionCall);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}

	}
);

/// Mutate expression to variable reference
static YulProtoMutator mutateExprToVarRef(
	Expression::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_highIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Mutate expression to variable ref" << std::endl;
#endif
			auto expr = static_cast<Expression*>(_message);
			YulProtoMutator::clearExpr(expr);
			expr->set_allocated_varref(YulProtoMutator::varRef(_seed/19));
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add varref to statement
static YulProtoMutator addVarRefToStmt(
	Statement::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_highIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add varref to statement" << std::endl;
#endif
			auto stmt = static_cast<Statement*>(_message);
			YulProtoMutator::addArgs(stmt, _seed, YulProtoMutator::refExpression);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add varrefs to statement arguments recursively
/// Add varref to statement
static YulProtoMutator addVarRefToStmtRec(
	Statement::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_highIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add recursive varref to statement" << std::endl;
#endif
			auto stmt = static_cast<Statement*>(_message);
			YulProtoMutator::addArgsRec(stmt, _seed, YulProtoMutator::initOrVarRef);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add binop expression to statement
static YulProtoMutator addBinopToStmt(
	Statement::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_highIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add binop to statement" << std::endl;
#endif
			auto stmt = static_cast<Statement*>(_message);
			YulProtoMutator::addArgs(stmt, _seed/7, YulProtoMutator::binopExpression);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

/// Add load expression to statement
static YulProtoMutator addLoadToStmt(
	Statement::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_highIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add load to statement" << std::endl;
#endif
			auto stmt = static_cast<Statement*>(_message);
			YulProtoMutator::addArgs(stmt, _seed, YulProtoMutator::loadExpression);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

static YulProtoMutator addStmt(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_highIP == 0)
		{
#ifdef DEBUG
			std::cout << "----------------------------------" << std::endl;
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add statement to block" << std::endl;
#endif
			auto block = static_cast<Block*>(_message);
			YulProtoMutator::addStmt(block, _seed);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "----------------------------------" << std::endl;
#endif
		}
	}
);

void YulProtoMutator::addArgs(
	Statement *_stmt,
	unsigned int _seed,
	std::function<Expression *(unsigned int)> _func
)
{
	switch (_stmt->stmt_oneof_case())
	{
	case Statement::kDecl:
		if (!_stmt->decl().has_expr() || _stmt->decl().expr().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_decl()->set_allocated_expr(_func(_seed/13));
		break;
	case Statement::kAssignment:
		if (!_stmt->assignment().has_expr() || _stmt->assignment().expr().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_assignment()->set_allocated_expr(_func(_seed/17));
		break;
	case Statement::kIfstmt:
		if (!_stmt->ifstmt().has_cond() || _stmt->ifstmt().cond().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_ifstmt()->set_allocated_cond(_func(_seed/23));
		break;
	case Statement::kStorageFunc:
		if (!_stmt->storage_func().has_loc() || _stmt->storage_func().loc().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_storage_func()->set_allocated_loc(_func(_seed/29));
		if (!_stmt->storage_func().has_val() || _stmt->storage_func().val().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_storage_func()->set_allocated_val(_func(_seed/37));
		break;
	case Statement::kBlockstmt:
		break;
	case Statement::kForstmt:
		if (!_stmt->forstmt().has_for_cond() || _stmt->forstmt().for_cond().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_forstmt()->set_allocated_for_cond(_func(_seed/41));
		break;
	case Statement::kBoundedforstmt:
		break;
	case Statement::kSwitchstmt:
		if (!_stmt->switchstmt().has_switch_expr() || _stmt->switchstmt().switch_expr().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_switchstmt()->set_allocated_switch_expr(_func(_seed/43));
		break;
	case Statement::kBreakstmt:
		break;
	case Statement::kContstmt:
		break;
	case Statement::kLogFunc:
		if (!_stmt->log_func().has_pos() || _stmt->log_func().pos().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_log_func()->set_allocated_pos(_func(_seed/19));
		if (!_stmt->log_func().has_size() || _stmt->log_func().size().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_log_func()->set_allocated_size(_func(_seed/17));
		if (!_stmt->log_func().has_t1() || _stmt->log_func().t1().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_log_func()->set_allocated_t1(_func(_seed/7));
		if (!_stmt->log_func().has_t2() || _stmt->log_func().t2().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_log_func()->set_allocated_t2(_func(_seed/5));
		if (!_stmt->log_func().has_t3() || _stmt->log_func().t3().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_log_func()->set_allocated_t3(_func(_seed/3));
		if (!_stmt->log_func().has_t4() || _stmt->log_func().t4().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_log_func()->set_allocated_t4(_func(_seed));
		break;
	case Statement::kCopyFunc:
		if (!_stmt->copy_func().has_target() || _stmt->copy_func().target().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_copy_func()->set_allocated_target(_func(_seed/17));
		if (!_stmt->copy_func().has_source() || _stmt->copy_func().source().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_copy_func()->set_allocated_source(_func(_seed/13));
		if (!_stmt->copy_func().has_size() || _stmt->copy_func().size().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_copy_func()->set_allocated_size(_func(_seed/11));
		break;
	case Statement::kExtcodeCopy:
		if (!_stmt->extcode_copy().has_addr() || _stmt->extcode_copy().addr().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_extcode_copy()->set_allocated_addr(_func(_seed/19));
		if (!_stmt->extcode_copy().has_target() || _stmt->extcode_copy().target().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_extcode_copy()->set_allocated_target(_func(_seed/17));
		if (!_stmt->extcode_copy().has_source() || _stmt->extcode_copy().source().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_extcode_copy()->set_allocated_source(_func(_seed/13));
		if (!_stmt->extcode_copy().has_size() || _stmt->extcode_copy().size().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_extcode_copy()->set_allocated_size(_func(_seed/11));
		break;
	case Statement::kTerminatestmt:
		break;
	case Statement::kFunctioncall:
		if (!_stmt->functioncall().has_in_param1() || _stmt->functioncall().in_param1().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_functioncall()->set_allocated_in_param1(_func(_seed/101));
		if (!_stmt->functioncall().has_in_param2() || _stmt->functioncall().in_param2().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_functioncall()->set_allocated_in_param2(_func(_seed/103));
		if (!_stmt->functioncall().has_in_param3() || _stmt->functioncall().in_param3().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_functioncall()->set_allocated_in_param3(_func(_seed/107));
		if (!_stmt->functioncall().has_in_param4() || _stmt->functioncall().in_param4().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_functioncall()->set_allocated_in_param4(_func(_seed/113));
		break;
	case Statement::kFuncdef:
		break;
	case Statement::kPop:
		if (!_stmt->pop().has_expr() || _stmt->pop().expr().expr_oneof_case() == Expression::EXPR_ONEOF_NOT_SET)
			_stmt->mutable_pop()->set_allocated_expr(_func(_seed/51));
		break;
	case Statement::kLeave:
		break;
	case Statement::kMultidecl:
		break;
	case Statement::STMT_ONEOF_NOT_SET:
		break;
	}
}

void YulProtoMutator::addArgsRec(
	Statement *_stmt,
	unsigned int _seed,
	std::function<void(Expression*, unsigned int)> _func
)
{
	switch (_stmt->stmt_oneof_case())
	{
	case Statement::kDecl:
		_func(_stmt->mutable_decl()->mutable_expr(), _seed/13);
		break;
	case Statement::kAssignment:
		_func(_stmt->mutable_assignment()->mutable_expr(), _seed/17);
		break;
	case Statement::kIfstmt:
		_func(_stmt->mutable_ifstmt()->mutable_cond(), _seed/23);
		break;
	case Statement::kStorageFunc:
		_func(_stmt->mutable_storage_func()->mutable_loc(), _seed/29);
		_func(_stmt->mutable_storage_func()->mutable_val(), _seed/37);
		break;
	case Statement::kBlockstmt:
		break;
	case Statement::kForstmt:
		_func(_stmt->mutable_forstmt()->mutable_for_cond(), _seed/41);
		break;
	case Statement::kBoundedforstmt:
		break;
	case Statement::kSwitchstmt:
		_func(_stmt->mutable_switchstmt()->mutable_switch_expr(), _seed/43);
		break;
	case Statement::kBreakstmt:
		break;
	case Statement::kContstmt:
		break;
	case Statement::kLogFunc:
		_func(_stmt->mutable_log_func()->mutable_pos(), _seed/19);
		_func(_stmt->mutable_log_func()->mutable_size(), _seed/17);
		_func(_stmt->mutable_log_func()->mutable_t1(), _seed/7);
		_func(_stmt->mutable_log_func()->mutable_t2(), _seed/5);
		_func(_stmt->mutable_log_func()->mutable_t3(), _seed/3);
		_func(_stmt->mutable_log_func()->mutable_t4(), _seed);
		break;
	case Statement::kCopyFunc:
		_func(_stmt->mutable_copy_func()->mutable_target(), _seed/17);
		_func(_stmt->mutable_copy_func()->mutable_source(), _seed/13);
		_func(_stmt->mutable_copy_func()->mutable_size(), _seed/11);
		break;
	case Statement::kExtcodeCopy:
		_func(_stmt->mutable_extcode_copy()->mutable_addr(), _seed/19);
		_func(_stmt->mutable_extcode_copy()->mutable_target(), _seed/17);
		_func(_stmt->mutable_extcode_copy()->mutable_source(), _seed/13);
		_func(_stmt->mutable_extcode_copy()->mutable_size(), _seed/11);
		break;
	case Statement::kTerminatestmt:
		if (_stmt->terminatestmt().term_oneof_case() == TerminatingStmt::kRetRev)
		{
			_func(_stmt->mutable_terminatestmt()->mutable_ret_rev()->mutable_pos(), _seed/11);
			_func(_stmt->mutable_terminatestmt()->mutable_ret_rev()->mutable_size(), _seed/13);
		}
		else if (_stmt->terminatestmt().term_oneof_case() == TerminatingStmt::kSelfDes)
			_func(_stmt->mutable_terminatestmt()->mutable_self_des()->mutable_addr(), _seed/17);
		break;
	case Statement::kFunctioncall:
		_func(_stmt->mutable_functioncall()->mutable_in_param1(), _seed/11);
		_func(_stmt->mutable_functioncall()->mutable_in_param2(), _seed/13);
		_func(_stmt->mutable_functioncall()->mutable_in_param3(), _seed/17);
		_func(_stmt->mutable_functioncall()->mutable_in_param4(), _seed/19);
		break;
	case Statement::kFuncdef:
		break;
	case Statement::kPop:
		_func(_stmt->mutable_pop()->mutable_expr(), _seed/11);
		break;
	case Statement::kLeave:
		break;
	case Statement::kMultidecl:
		break;
	case Statement::STMT_ONEOF_NOT_SET:
		break;
	}
}

void YulProtoMutator::addStmt(Block* _block, unsigned _seed)
{
	auto stmt = _block->add_statements();
	switch ((_seed / 17) % 19)
	{
	case 0:
		stmt->set_allocated_decl(new VarDecl());
		break;
	case 1:
		stmt->set_allocated_assignment(new AssignmentStatement());
		break;
	case 2:
		stmt->set_allocated_ifstmt(new IfStmt());
		break;
	case 3:
		stmt->set_allocated_storage_func(new StoreFunc());
		break;
	case 4:
		stmt->set_allocated_blockstmt(new Block());
		break;
	case 5:
		stmt->set_allocated_forstmt(new ForStmt());
		break;
	case 6:
		stmt->set_allocated_switchstmt(new SwitchStmt());
		break;
	case 7:
		stmt->set_allocated_breakstmt(new BreakStmt());
		break;
	case 8:
		stmt->set_allocated_contstmt(new ContinueStmt());
		break;
	case 9:
		stmt->set_allocated_log_func(new LogFunc());
		break;
	case 10:
		stmt->set_allocated_copy_func(new CopyFunc());
		break;
	case 11:
		stmt->set_allocated_extcode_copy(new ExtCodeCopy());
		break;
	case 12:
		stmt->set_allocated_terminatestmt(new TerminatingStmt());
		break;
	case 13:
		stmt->set_allocated_functioncall(new FunctionCall());
		break;
	case 14:
		stmt->set_allocated_boundedforstmt(new BoundedForStmt());
		break;
	case 15:
		stmt->set_allocated_funcdef(new FunctionDef());
		break;
	case 16:
		stmt->set_allocated_pop(new PopStmt());
		break;
	case 17:
		stmt->set_allocated_leave(new LeaveStmt());
		break;
	case 18:
		stmt->set_allocated_multidecl(new MultiVarDecl());
		break;
	}
}


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
	auto refExpr = new Expression();
	refExpr->set_allocated_varref(varRef(_seed));
	return refExpr;
}

void YulProtoMutator::configureCall(FunctionCall *_call, unsigned int _seed)
{
	auto type = EnumTypeConverter<FunctionCall_Returns>{}.enumFromSeed(_seed);
	_call->set_ret(type);
	_call->set_func_index(_seed);
	configureCallArgs(type, _call, _seed);
}

void YulProtoMutator::configureCallArgs(
	FunctionCall_Returns _callType,
	FunctionCall *_call,
	unsigned _seed
)
{
	// Configuration rules:
	// All function calls must configure four input arguments, because
	// a function of any type may have at most four input arguments.
	// Out arguments need to be configured only for multi-assign
	switch (_callType)
	{
	case FunctionCall_Returns_MULTIASSIGN:
	{
		auto outRef4 = YulProtoMutator::varRef(_seed/8);
		_call->set_allocated_out_param4(outRef4);

		auto outRef3 = YulProtoMutator::varRef(_seed/7);
		_call->set_allocated_out_param3(outRef3);

		auto outRef2 = YulProtoMutator::varRef(_seed/6);
		_call->set_allocated_out_param2(outRef2);

		auto outRef1 = YulProtoMutator::varRef(_seed/5);
		_call->set_allocated_out_param1(outRef1);
	}
	BOOST_FALLTHROUGH;
	case FunctionCall_Returns_MULTIDECL:
	BOOST_FALLTHROUGH;
	case FunctionCall_Returns_SINGLE:
	BOOST_FALLTHROUGH;
	case FunctionCall_Returns_ZERO:
	{
		auto inArg4 = new Expression();
		auto inRef4 = YulProtoMutator::varRef(_seed/4);
		inArg4->set_allocated_varref(inRef4);
		_call->set_allocated_in_param4(inArg4);

		auto inArg3 = new Expression();
		auto inRef3 = YulProtoMutator::varRef(_seed/3);
		inArg3->set_allocated_varref(inRef3);
		_call->set_allocated_in_param3(inArg3);

		auto inArg2 = new Expression();
		auto inRef2 = YulProtoMutator::varRef(_seed/2);
		inArg2->set_allocated_varref(inRef2);
		_call->set_allocated_in_param2(inArg2);

		auto inArg1 = new Expression();
		auto inRef1 = YulProtoMutator::varRef(_seed);
		inArg1->set_allocated_varref(inRef1);
		_call->set_allocated_in_param1(inArg1);
		break;
	}
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
	else if constexpr (std::is_same_v<std::decay_t<T>, LowLevelCall_Type>)
		yulAssert(LowLevelCall_Type_IsValid(ret), "Yul proto mutator: Invalid enum");
	else if constexpr (std::is_same_v<std::decay_t<T>, Create_Type>)
		yulAssert(Create_Type_IsValid(ret), "Yul proto mutator: Invalid enum");
	else if constexpr (std::is_same_v<std::decay_t<T>, UnaryOpData_UOpData>)
		yulAssert(UnaryOpData_UOpData_IsValid(ret), "Yul proto mutator: Invalid enum");
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
	else if constexpr (std::is_same_v<std::decay_t<T>, LowLevelCall_Type>)
		return LowLevelCall_Type_Type_MAX;
	else if constexpr (std::is_same_v<std::decay_t<T>, Create_Type>)
		return Create_Type_Type_MAX;
	else if constexpr (std::is_same_v<std::decay_t<T>, UnaryOpData_UOpData>)
		return UnaryOpData_UOpData_UOpData_MAX;
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
	else if constexpr (std::is_same_v<std::decay_t<T>, LowLevelCall_Type>)
		return LowLevelCall_Type_Type_MIN;
	else if constexpr (std::is_same_v<std::decay_t<T>, Create_Type>)
		return Create_Type_Type_MIN;
	else if constexpr (std::is_same_v<std::decay_t<T>, UnaryOpData_UOpData>)
		return UnaryOpData_UOpData_UOpData_MIN;
	else
		static_assert(AlwaysFalse<T>::value, "Yul proto mutator: non-exhaustive visitor.");
}

Expression* YulProtoMutator::loadExpression(unsigned _seed)
{
	auto unop = new UnaryOp();
	unop->set_allocated_operand(refExpression(_seed/17));
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
	auto expr = new Expression();
	expr->set_allocated_unop(unop);
	return expr;
}

void YulProtoMutator::clearExpr(Expression* _expr)
{
	switch (_expr->expr_oneof_case())
	{
	case Expression::kVarref:
		delete _expr->release_varref();
		_expr->clear_varref();
		break;
	case Expression::kCons:
		delete _expr->release_cons();
		_expr->clear_cons();
		break;
	case Expression::kBinop:
		delete _expr->release_binop();
		_expr->clear_binop();
		break;
	case Expression::kUnop:
		delete _expr->release_unop();
		_expr->clear_unop();
		break;
	case Expression::kTop:
		delete _expr->release_top();
		_expr->clear_top();
		break;
	case Expression::kNop:
		delete _expr->release_nop();
		_expr->clear_nop();
		break;
	case Expression::kFuncExpr:
		delete _expr->release_func_expr();
		_expr->clear_func_expr();
		break;
	case Expression::kLowcall:
		delete _expr->release_lowcall();
		_expr->clear_lowcall();
		break;
	case Expression::kCreate:
		delete _expr->release_create();
		_expr->clear_create();
		break;
	case Expression::kUnopdata:
		delete _expr->release_unopdata();
		_expr->clear_unopdata();
		break;
	case Expression::EXPR_ONEOF_NOT_SET:
		break;
	}
}

Expression* YulProtoMutator::binopExpression(unsigned _seed)
{
	auto binop = new BinaryOp();
	binop->set_allocated_left(refExpression(_seed/17));
	binop->set_allocated_right(refExpression(_seed/21));
	binop->set_op(
		YulProtoMutator::EnumTypeConverter<BinaryOp_BOp>{}.enumFromSeed(_seed/23)
	);
	auto expr = new Expression();
	expr->set_allocated_binop(binop);
	return expr;
}

void YulProtoMutator::initOrVarRef(Expression* _expr, unsigned _seed)
{
	switch (_expr->expr_oneof_case())
	{
	// Nothing to be done because expression does not
	// contain sub-expression.
	case Expression::kVarref:
		break;
	case Expression::kCons:
		if (_expr->cons().literal_oneof_case() == Literal::LITERAL_ONEOF_NOT_SET)
			_expr->mutable_cons()->set_intval(_seed);
		break;
	case Expression::kBinop:
		if (!set(_expr->binop().left()))
			_expr->mutable_binop()->mutable_left()->set_allocated_varref(varRef(_seed));
		else
			initOrVarRef(_expr->mutable_binop()->mutable_left(), _seed);

		if (!set(_expr->binop().right()))
			_expr->mutable_binop()->mutable_right()->set_allocated_varref(varRef(_seed/17));
		else
			initOrVarRef(_expr->mutable_binop()->mutable_right(), _seed);
		break;
	case Expression::kUnop:
		if (!set(_expr->unop().operand()))
			_expr->mutable_unop()->mutable_operand()->set_allocated_varref(varRef(_seed));
		else
			initOrVarRef(_expr->mutable_unop()->mutable_operand(), _seed);
		break;
	case Expression::kTop:
		if (!set(_expr->top().arg1()))
			_expr->mutable_top()->mutable_arg1()->set_allocated_varref(varRef(_seed));
		else
			initOrVarRef(_expr->mutable_top()->mutable_arg1(), _seed);

		if (!set(_expr->top().arg2()))
			_expr->mutable_top()->mutable_arg2()->set_allocated_varref(varRef(_seed/17));
		else
			initOrVarRef(_expr->mutable_top()->mutable_arg2(), _seed);

		if (!set(_expr->top().arg3()))
			_expr->mutable_top()->mutable_arg3()->set_allocated_varref(varRef(_seed));
		else
			initOrVarRef(_expr->mutable_top()->mutable_arg3(), _seed);
		break;
	case Expression::kNop:
		break;
	case Expression::kFuncExpr:
		_expr->mutable_func_expr()->set_ret(FunctionCall_Returns::FunctionCall_Returns_SINGLE);

		if (!set(_expr->func_expr().in_param1()))
			_expr->mutable_func_expr()->mutable_in_param1()->set_allocated_varref(varRef(_seed));
		else
			initOrVarRef(_expr->mutable_func_expr()->mutable_in_param1(), _seed);

		if (!set(_expr->func_expr().in_param2()))
			_expr->mutable_func_expr()->mutable_in_param2()->set_allocated_varref(varRef(_seed/7));
		else
			initOrVarRef(_expr->mutable_func_expr()->mutable_in_param2(), _seed);

		if (!set(_expr->func_expr().in_param3()))
			_expr->mutable_func_expr()->mutable_in_param3()->set_allocated_varref(varRef(_seed/11));
		else
			initOrVarRef(_expr->mutable_func_expr()->mutable_in_param3(), _seed);

		if (!set(_expr->func_expr().in_param4()))
			_expr->mutable_func_expr()->mutable_in_param4()->set_allocated_varref(varRef(_seed));
		else
			initOrVarRef(_expr->mutable_func_expr()->mutable_in_param4(), _seed);

		break;
	case Expression::kLowcall:
		// Wei
		if (_expr->lowcall().callty() == LowLevelCall::CALLCODE || _expr->lowcall().callty() == LowLevelCall::CALL)
		{
			if (!set(_expr->lowcall().wei()))
				_expr->mutable_lowcall()->mutable_wei()->set_allocated_varref(varRef(_seed));
			else
				initOrVarRef(_expr->mutable_lowcall()->mutable_wei(), _seed);
		}

		// Gas
		if (!set(_expr->lowcall().gas()))
			_expr->mutable_lowcall()->mutable_gas()->set_allocated_varref(varRef(_seed/7));
		else
			initOrVarRef(_expr->mutable_lowcall()->mutable_gas(), _seed);

		// Addr
		if (!set(_expr->lowcall().addr()))
			_expr->mutable_lowcall()->mutable_addr()->set_allocated_varref(varRef(_seed/5));
		else
			initOrVarRef(_expr->mutable_lowcall()->mutable_addr(), _seed);

		// In
		if (!set(_expr->lowcall().in()))
			_expr->mutable_lowcall()->mutable_in()->set_allocated_varref(varRef(_seed/3));
		else
			initOrVarRef(_expr->mutable_lowcall()->mutable_in(), _seed);
		// Insize
		if (!set(_expr->lowcall().insize()))
			_expr->mutable_lowcall()->mutable_insize()->set_allocated_varref(varRef(_seed/11));
		else
			initOrVarRef(_expr->mutable_lowcall()->mutable_insize(), _seed);
		// Out
		if (!set(_expr->lowcall().out()))
			_expr->mutable_lowcall()->mutable_out()->set_allocated_varref(varRef(_seed/13));
		else
			initOrVarRef(_expr->mutable_lowcall()->mutable_out(), _seed);
		// Outsize
		if (!set(_expr->lowcall().outsize()))
			_expr->mutable_lowcall()->mutable_outsize()->set_allocated_varref(varRef(_seed/17));
		else
			initOrVarRef(_expr->mutable_lowcall()->mutable_outsize(), _seed);
		break;
	case Expression::kCreate:
		// Value
		if (_expr->create().createty() == Create_Type::Create_Type_CREATE2)
		{
			if (!set(_expr->create().value()))
				_expr->mutable_create()->mutable_value()->set_allocated_varref(varRef(_seed/5));
			else
				initOrVarRef(_expr->mutable_create()->mutable_value(), _seed);
		}
		// Wei
		if (!set(_expr->create().wei()))
			_expr->mutable_create()->mutable_wei()->set_allocated_varref(varRef(_seed/7));
		else
			initOrVarRef(_expr->mutable_create()->mutable_wei(), _seed);
		// Position
		if (!set(_expr->create().position()))
			_expr->mutable_create()->mutable_position()->set_allocated_varref(varRef(_seed/11));
		else
			initOrVarRef(_expr->mutable_create()->mutable_position(), _seed);
		// Size
		if (!set(_expr->create().size()))
			_expr->mutable_create()->mutable_size()->set_allocated_varref(varRef(_seed/13));
		else
			initOrVarRef(_expr->mutable_create()->mutable_size(), _seed);
		break;
	case Expression::kUnopdata:
		break;
	case Expression::EXPR_ONEOF_NOT_SET:
		_expr->set_allocated_varref(varRef(_seed/17));
		break;
	}
}