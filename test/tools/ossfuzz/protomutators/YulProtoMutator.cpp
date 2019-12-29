#include <test/tools/ossfuzz/protomutators/YulProtoMutator.h>

#include <src/text_format.h>

#define DEBUG

using namespace yul::test::yul_fuzzer;

/// Invert condition of an if statement
static YulProtoMutator invertIfCondition(
	IfStmt::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		IfStmt* ifStmt = static_cast<IfStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (ifStmt->has_cond())
			{
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: If condition inverted" << std::endl;
#endif
				UnaryOp* notOp = new UnaryOp();
				notOp->set_op(UnaryOp::NOT);
				Expression *oldCond = ifStmt->release_cond();
				notOp->set_allocated_operand(oldCond);
				Expression *ifCond = new Expression();
				ifCond->set_allocated_unop(notOp);
				ifStmt->set_allocated_cond(ifCond);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
		IfStmt* ifStmt = static_cast<IfStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (ifStmt->has_cond() &&
				ifStmt->cond().has_unop() &&
				ifStmt->cond().unop().has_op() &&
				ifStmt->cond().unop().op() == UnaryOp::NOT
			)
			{
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Remove If condition inverted" << std::endl;
#endif
				Expression *oldCondition = ifStmt->release_cond();
				UnaryOp *unop = oldCondition->release_unop();
				Expression *conditionWithoutNot = unop->release_operand();
				ifStmt->set_allocated_cond(conditionWithoutNot);
				delete(oldCondition);
				delete(unop);
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
		ForStmt* forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (forStmt->has_for_body())
			{
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Break added" << std::endl;
#endif
				BreakStmt* breakStmt = new BreakStmt();
				Statement* statement = forStmt->mutable_for_body()->add_statements();
				statement->set_allocated_breakstmt(breakStmt);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
		ForStmt* forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (forStmt->has_for_body())
			{
#ifdef DEBUG
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
		ForStmt* forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (forStmt->has_for_body())
			{
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Continue added" << std::endl;
#endif
				ContinueStmt* contStmt = new ContinueStmt();
				Statement* statement = forStmt->mutable_for_body()->add_statements();
				statement->set_allocated_contstmt(contStmt);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
		ForStmt* forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (forStmt->has_for_body())
			{
#ifdef DEBUG
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
		VarDecl* varDeclStmt = static_cast<VarDecl*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (varDeclStmt->has_expr())
			{
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: mload added" << std::endl;
#endif
				varDeclStmt->clear_expr();
				Literal *zeroLit = new Literal();
				zeroLit->set_intval(0);
				Expression *consExpr = new Expression();
				consExpr->set_allocated_cons(zeroLit);
				UnaryOp *mloadOp = new UnaryOp();
				mloadOp->set_op(UnaryOp::MLOAD);
				mloadOp->set_allocated_operand(consExpr);
				Expression *mloadExpr = new Expression();
				mloadExpr->set_allocated_unop(mloadOp);
				varDeclStmt->set_allocated_expr(mloadExpr);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
		ForStmt* forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (forStmt->has_for_cond())
			{
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: For condition inverted" << std::endl;
#endif
				UnaryOp* notOp = new UnaryOp();
				notOp->set_op(UnaryOp::NOT);
				Expression *oldCond = forStmt->release_for_cond();
				notOp->set_allocated_operand(oldCond);
				Expression *forCond = new Expression();
				forCond->set_allocated_unop(notOp);
				forStmt->set_allocated_for_cond(forCond);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
		ForStmt* forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 1)
		{
			if (forStmt->has_for_cond() &&
				forStmt->for_cond().has_unop() &&
				forStmt->for_cond().unop().has_op() &&
				forStmt->for_cond().unop().op() == UnaryOp::NOT
			)
			{
#ifdef DEBUG
				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Remove For condition inverted" << std::endl;
#endif
				Expression *oldCondition = forStmt->release_for_cond();
				UnaryOp *unop = oldCondition->release_unop();
				Expression *newCondition = unop->release_operand();
				forStmt->set_allocated_for_cond(newCondition);
				delete oldCondition;
				delete unop;
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
		ForStmt* forStmt = static_cast<ForStmt*>(_message);
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
			if (forStmt->has_for_cond())
			{
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
				std::cout << "YULMUTATOR: Function call in for condition" << std::endl;
#endif
				forStmt->release_for_cond();
				FunctionCall *functionCall = new FunctionCall();
				functionCall->set_ret(FunctionCall::SINGLE);
				functionCall->set_func_index(0);
				Expression *forCondExpr = new Expression();
				forCondExpr->set_allocated_func_expr(functionCall);
				forStmt->set_allocated_for_cond(forCondExpr);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
#endif
			}
		}
	}
);

/// Define an identity function f(x) = x
static YulProtoMutator identityFunction(
	Block::descriptor(),
	[](google::protobuf::Message* _message, unsigned int _seed)
	{
		if (_seed % YulProtoMutator::s_mediumIP == 0)
		{
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Identity function" << std::endl;
#endif
			Block* blockStmt = static_cast<Block*>(_message);
			FunctionDef* functionDef = new FunctionDef();
			functionDef->set_num_input_params(1);
			functionDef->set_num_output_params(1);
			Block* functionBlock = new Block();
			AssignmentStatement* assignmentStatement = new AssignmentStatement();
			VarRef* varRef = new VarRef();
			varRef->set_varnum(1);
			assignmentStatement->set_allocated_ref_id(varRef);
			Expression* rhs = new Expression();
			VarRef* rhsRef = new VarRef();
			rhsRef->set_varnum(0);
			rhs->set_allocated_varref(rhsRef);
			assignmentStatement->set_allocated_expr(rhs);
			Statement *stmt = functionBlock->add_statements();
			stmt->set_allocated_assignment(assignmentStatement);
			functionDef->set_allocated_block(functionBlock);
			Statement *funcdefStmt = blockStmt->add_statements();
			funcdefStmt->set_allocated_funcdef(functionDef);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Leave in function" << std::endl;
#endif
			FunctionDef *funcDef = static_cast<FunctionDef*>(_message);
			Statement *newStmt = funcDef->mutable_block()->add_statements();
			LeaveStmt *leaveStmt = new LeaveStmt();
			newStmt->set_allocated_leave(leaveStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove Leave in function" << std::endl;
#endif
			FunctionDef *funcDef = static_cast<FunctionDef*>(_message);
			for (auto &stmt: *funcDef->mutable_block()->mutable_statements())
				if (stmt.has_leave())
				{
					stmt.clear_leave();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add assignment" << std::endl;
#endif
			Block *block = static_cast<Block*>(_message);
			AssignmentStatement *assignmentStatement = new AssignmentStatement();
			VarRef *varRef = new VarRef();
			varRef->set_varnum(0);
			assignmentStatement->set_allocated_ref_id(varRef);
			VarRef *rhs = new VarRef();
			rhs->set_varnum(1);
			Expression *rhsExpr = new Expression();
			rhsExpr->set_allocated_varref(rhs);
			assignmentStatement->set_allocated_expr(rhsExpr);
			Statement *newStmt = block->add_statements();
			newStmt->set_allocated_assignment(assignmentStatement);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove assignment" << std::endl;
#endif
			Block *block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_assignment())
				{
					stmt.clear_assignment();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add if" << std::endl;
#endif
			Block *block = static_cast<Block*>(_message);
			Statement *stmt = block->add_statements();
			IfStmt *ifStmt = new IfStmt();
			stmt->set_allocated_ifstmt(ifStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove if" << std::endl;
#endif
			Block *block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_ifstmt())
				{
					stmt.clear_ifstmt();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Add switch" << std::endl;
#endif
			Block *block = static_cast<Block*>(_message);
			Statement *stmt = block->add_statements();
			SwitchStmt *switchStmt = new SwitchStmt();
			stmt->set_allocated_switchstmt(switchStmt);
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
			std::cout << "YULMUTATOR: Remove switch" << std::endl;
#endif
			Block *block = static_cast<Block*>(_message);
			for (auto &stmt: *block->mutable_statements())
				if (stmt.has_switchstmt())
				{
					stmt.clear_switchstmt();
					break;
				}
#ifdef DEBUG
			std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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

VarRef* YulProtoMutator::varRef(unsigned _index)
{
	VarRef *varref = new VarRef();
	varref->set_varnum(_index);
	return varref;
}