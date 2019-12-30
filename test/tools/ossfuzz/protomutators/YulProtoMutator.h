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
	/// @param _seed: Pseudo-random unsigned integer used to reference
	/// an existing variable.
	static VarRef* varRef(unsigned _seed);

	/// Return a valid function call type from a pseudo-random seed.
	/// @param _seed: Pseudo-random unsigned integer
	static FunctionCall_Returns callType(unsigned _seed);

	/// Configure function call from a pseudo-random seed.
	/// @param _call: Pre-allocated FunctionCall protobuf message
	/// @param _seed: Pseudo-random unsigned integer
	static void configureCall(FunctionCall *_call, unsigned _seed);

	static constexpr unsigned s_lowIP = 827;
	static constexpr unsigned s_mediumIP = 569;
	static constexpr unsigned s_highIP = 251;
};



}
}
}