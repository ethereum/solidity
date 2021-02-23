#pragma once

#include <test/tools/ossfuzz/yulProto.pb.h>

#include <libsolutil/Common.h>

#include <src/libfuzzer/libfuzzer_macro.h>

#include <random>

namespace solidity::yul::test::yul_fuzzer
{

using ProtobufMessage = google::protobuf::Message;

template <typename Proto>
using LPMPostProcessor = protobuf_mutator::libfuzzer::PostProcessorRegistration<Proto>;

class MutationInfo: public ScopeGuard
{
public:
	MutationInfo(ProtobufMessage const* _message, std::string const& _info);

	static void writeLine(std::string const& _str)
	{
		std::cout << _str << std::endl;
	}
	void exitInfo();

	ProtobufMessage const* m_protobufMsg;
};

struct YulRandomNumGenerator
{
	using RandomEngine = std::minstd_rand;

	explicit YulRandomNumGenerator(unsigned _seed): m_random(RandomEngine(_seed)) {}

	unsigned operator()()
	{
		return static_cast<unsigned>(m_random());
	}

	RandomEngine m_random;
};

struct YulProtoMutator
{
	/// @param _value: Value of the integer literal
	/// @returns an integer literal protobuf message initialized with
	/// the given value.
	static Literal* intLiteral(unsigned _value);

	/// @param _seed: Pseudo-random unsigned integer used as index
	/// of variable to be referenced
	/// @returns a variable reference protobuf message.
	static VarRef* varRef(unsigned _seed);

	/// @param _value: value of literal expression
	/// @returns an expression protobuf message
	static Expression* litExpression(unsigned _value);

	/// @param _rand: Pseudo-random number generator
	/// of variable to be referenced
	/// @returns a variable reference protobuf message
	static Expression* refExpression(YulRandomNumGenerator& _rand);

	/// Helper type for type matching visitor.
	template<class T> struct AlwaysFalse: std::false_type {};

	/// Template struct for obtaining a valid enum value of
	/// template type from a pseudo-random unsigned integer.
	/// @param _seed: Pseudo-random integer
	/// @returns Valid enum of enum type T
	template <typename T>
	struct EnumTypeConverter
	{
		T enumFromSeed(unsigned _seed)
		{
			return validEnum(_seed);
		}

		/// @returns a valid enum of type T from _seed
		T validEnum(unsigned _seed);
		/// @returns maximum enum value for enum of type T
		static unsigned enumMax();
		/// @returns minimum enum value for enum of type T
		static unsigned enumMin();
	};

	/// Modulo for mutations that should occur rarely
	static constexpr unsigned s_lowIP = 31;
	/// Modulo for mutations that should occur not too often
	static constexpr unsigned s_mediumIP = 29;
	/// Modulo for mutations that should occur often
	static constexpr unsigned s_highIP = 23;
	/// Add control-flow statement to basic block.
	template <typename T>
	static void addControlFlow(T* _msg);
	/// Obtain basic block for statement type.
	template <typename T>
	static Block* basicBlock(T* _msg);
	/// Obtain a basic block in a for stmt uniformly
	/// at random
	static Block* randomBlock(ForStmt* _msg);
};
}
