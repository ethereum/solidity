#include <test/tools/ossfuzz/protomutators/YulProtoMutator.h>

#include <libyul/Exceptions.h>

#include <src/text_format.h>

using namespace solidity::yul::test::yul_fuzzer;
using namespace protobuf_mutator;
using namespace std;

using YPM = YulProtoMutator;

MutationInfo::MutationInfo(ProtobufMessage const* _message, string const& _info):
	ScopeGuard([&]{ exitInfo(); }),
	m_protobufMsg(_message)
{
	writeLine("----------------------------------");
	writeLine("YULMUTATOR: " + _info);
	writeLine("Before");
	writeLine(SaveMessageAsText(*m_protobufMsg));
}

void MutationInfo::exitInfo()
{
	writeLine("After");
	writeLine(SaveMessageAsText(*m_protobufMsg));
}

/// Add m/sstore(0, variable)
static LPMPostProcessor<Block> addStoreToZero(
	[](Block* _message, unsigned _seed)
	{
		if (_seed % YPM::s_highIP == 0)
		{
			YulRandomNumGenerator yrand(_seed);
			MutationInfo m{_message, "Added store to zero"};
			auto storeStmt = new StoreFunc();
			storeStmt->set_st(YPM::EnumTypeConverter<StoreFunc_Storage>{}.enumFromSeed(yrand()));
			storeStmt->set_allocated_loc(YPM::litExpression(0));
			storeStmt->set_allocated_val(YPM::refExpression(yrand));
			auto stmt = _message->add_statements();
			stmt->set_allocated_storage_func(storeStmt);
		}
	}
);


namespace
{
template <typename T>
struct addControlFlow
{
	addControlFlow()
	{
		function = [](T* _message, unsigned _seed)
		{
			MutationInfo m{_message, "Added control flow."};
			YPM::addControlFlow(_message, _seed);
		};
		/// Unused variable registers callback.
		LPMPostProcessor<T> callback(function);
	}
	function<void(T*, unsigned)> function;
};
}

static addControlFlow<ForStmt> c1;
static addControlFlow<BoundedForStmt> c2;
static addControlFlow<IfStmt> c3;
static addControlFlow<SwitchStmt> c4;
static addControlFlow<FunctionDef> c5;
static addControlFlow<CaseStmt> c6;
static addControlFlow<Code> c7;
static addControlFlow<Program> c8;

Literal* YPM::intLiteral(unsigned _value)
{
	auto lit = new Literal();
	lit->set_intval(_value);
	return lit;
}

VarRef* YPM::varRef(unsigned _seed)
{
	auto varref = new VarRef();
	varref->set_varnum(_seed);
	return varref;
}

Expression* YPM::refExpression(YulRandomNumGenerator& _rand)
{
	auto refExpr = new Expression();
	refExpr->set_allocated_varref(varRef(_rand()));
	return refExpr;
}

Expression* YPM::litExpression(unsigned _value)
{
	auto lit = intLiteral(_value);
	auto expr = new Expression();
	expr->set_allocated_cons(lit);
	return expr;
}

template <typename T>
T YPM::EnumTypeConverter<T>::validEnum(unsigned _seed)
{
	auto ret = static_cast<T>(_seed % (enumMax() - enumMin() + 1) + enumMin());
	if constexpr (std::is_same_v<std::decay_t<T>, StoreFunc_Storage>)
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
unsigned YPM::EnumTypeConverter<T>::enumMax()
{
	if constexpr (std::is_same_v<std::decay_t<T>, StoreFunc_Storage>)
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
unsigned YPM::EnumTypeConverter<T>::enumMin()
{
	if constexpr (std::is_same_v<std::decay_t<T>, StoreFunc_Storage>)
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

template <typename T>
void YPM::addControlFlow(T* _msg, unsigned _seed)
{
	enum class ControlFlowStmt: unsigned
	{
		For = 0,
		BoundedFor,
		If,
		Switch,
		FunctionCall,
		Break,
		Continue,
		Leave,
		Termination
	};
	uniform_int_distribution<unsigned> d(
		static_cast<unsigned>(ControlFlowStmt::For),
		static_cast<unsigned>(ControlFlowStmt::Termination)
	);
	YulRandomNumGenerator yrand(_seed);
	auto random = static_cast<ControlFlowStmt>(d(yrand.m_random));
	Statement* s = basicBlock(_msg, _seed)->add_statements();
	switch (random)
	{
	case ControlFlowStmt::For:
		s->set_allocated_forstmt(new ForStmt());
		break;
	case ControlFlowStmt::BoundedFor:
		s->set_allocated_boundedforstmt(new BoundedForStmt());
		break;
	case ControlFlowStmt::If:
		s->set_allocated_ifstmt(new IfStmt());
		break;
	case ControlFlowStmt::Switch:
		s->set_allocated_switchstmt(new SwitchStmt());
		break;
	case ControlFlowStmt::FunctionCall:
		s->set_allocated_functioncall(new FunctionCall());
		break;
	case ControlFlowStmt::Break:
		s->set_allocated_breakstmt(new BreakStmt());
		break;
	case ControlFlowStmt::Continue:
		s->set_allocated_contstmt(new ContinueStmt());
		break;
	case ControlFlowStmt::Leave:
		s->set_allocated_leave(new LeaveStmt());
		break;
	case ControlFlowStmt::Termination:
		s->set_allocated_terminatestmt(new TerminatingStmt());
		break;
	}
}

Block* YPM::randomBlock(ForStmt* _stmt, unsigned _seed)
{
	enum class ForBlocks: unsigned
	{
		Init = 0,
		Post = 1,
		Body = 2
	};
	uniform_int_distribution<unsigned> d(
		static_cast<unsigned>(ForBlocks::Init),
		static_cast<unsigned>(ForBlocks::Body)
	);
	YulRandomNumGenerator yrand(_seed);
	switch (static_cast<ForBlocks>(d(yrand.m_random)))
	{
	case ForBlocks::Init:
		return _stmt->mutable_for_init();
	case ForBlocks::Post:
		return _stmt->mutable_for_post();
	case ForBlocks::Body:
		return _stmt->mutable_for_body();
	}
}

template <typename T>
Block* YPM::basicBlock(T* _msg, unsigned _seed)
{
	if constexpr (std::is_same_v<T, ForStmt>)
		return randomBlock(_msg, _seed);
	else if constexpr (std::is_same_v<T, BoundedForStmt>)
		return _msg->mutable_for_body();
	else if constexpr (std::is_same_v<T, SwitchStmt>)
		return _msg->mutable_default_block();
	else if constexpr (std::is_same_v<T, FunctionDef>)
		return _msg->mutable_block();
	else if constexpr (std::is_same_v<T, IfStmt>)
		return _msg->mutable_if_body();
	else if constexpr (std::is_same_v<T, CaseStmt>)
		return _msg->mutable_case_block();
	else if constexpr (std::is_same_v<T, Code>)
		return _msg->mutable_block();
	else if constexpr (std::is_same_v<T, Program>)
		return globalBlock(_msg);
	else
		static_assert(AlwaysFalse<T>::value, "Yul proto mutator: non-exhaustive visitor.");
}

Block* YPM::globalBlock(Program* _program)
{
	switch (_program->program_oneof_case())
	{
	case Program::kBlock:
		return _program->mutable_block();
	case Program::kObj:
		return _program->mutable_obj()->mutable_code()->mutable_block();
	case Program::PROGRAM_ONEOF_NOT_SET:
	{
		_program->set_allocated_block(new Block());
		return _program->mutable_block();
	}
	}
}
