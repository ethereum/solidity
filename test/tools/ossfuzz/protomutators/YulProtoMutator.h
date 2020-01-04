#pragma once

#include <test/tools/ossfuzz/yulProto.pb.h>

#include <src/libfuzzer/libfuzzer_macro.h>

namespace yul
{
namespace test
{
namespace yul_fuzzer
{
struct YulProtoMutator
{
	YulProtoMutator(
		google::protobuf::Descriptor const* _desc,
		std::function<void(google::protobuf::Message*, unsigned int)> _callback
	)
	{
		protobuf_mutator::libfuzzer::RegisterPostProcessor(_desc, _callback);
	}

	/// Return an integer literal of the given value.
	/// @param _value: Value of the integer literal
	static Literal* intLiteral(unsigned _value);

	/// Return a variable reference
	/// @param _seed: Pseudo-random unsigned integer used as index
	/// of variable to be referenced
	static VarRef* varRef(unsigned _seed);

	/// Return a literal expression
	/// @param _value: value of literal expression
	static Expression* litExpression(unsigned _value);

	/// Return a reference expression
	/// @param _seed: Pseudo-random unsigned integer used as index
	/// of variable to be referenced
	static Expression* refExpression(unsigned _seed);

	/// Configure function call from a pseudo-random seed.
	/// @param _call: Pre-allocated FunctionCall protobuf message
	/// @param _seed: Pseudo-random unsigned integer
	static void configureCall(FunctionCall *_call, unsigned _seed);

	/// Helper type for type matching visitor.
	template<class T> struct AlwaysFalse: std::false_type {};

	/// Template struct for obtaining a valid enum value of
	/// template type from pseudo-random unsigned integer.
	/// @param _seed: Pseudo-random integer
	/// @returns Valid enum of template enum type
	template <typename T>
	struct EnumTypeConverter
	{
		T enumFromSeed(unsigned _seed)
		{
			return validEnum(_seed);
		}

		/// Return a valid enum of type T from _seed
		T validEnum(unsigned _seed);
		/// Return maximum enum value for enum of type T
		static int enumMax();
		/// Return minimum enum value for enum of type T
		static int enumMin();
	};

	static constexpr unsigned s_lowIP = 827;
	static constexpr unsigned s_mediumIP = 569;
	static constexpr unsigned s_highIP = 251;
	static constexpr unsigned s_veryHighIP = 7;
};
}
}
}