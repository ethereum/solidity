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

	static constexpr unsigned s_lowIP = 41;
	static constexpr unsigned s_mediumIP = 29;
	static constexpr unsigned s_highIP = 17;
};



}
}
}