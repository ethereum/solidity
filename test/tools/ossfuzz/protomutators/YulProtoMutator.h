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
	/// @param _index: Index of a variable in scope, 0 being the first
	/// variable in scope.
	static VarRef* varRef(unsigned _index);

	static constexpr unsigned s_lowIP = 827;
	static constexpr unsigned s_mediumIP = 569;
	static constexpr unsigned s_highIP = 251;
};



}
}
}