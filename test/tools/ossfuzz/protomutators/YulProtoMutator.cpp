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
				statement->clear_stmt_oneof();
				statement->set_allocated_breakstmt(breakStmt);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
#endif
			}
		}
	}
);

/// Add break statement in body of a for-loop statement
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
				statement->clear_stmt_oneof();
				statement->set_allocated_contstmt(contStmt);
#ifdef DEBUG
//				std::cout << protobuf_mutator::SaveMessageAsText(*_message) << std::endl;
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