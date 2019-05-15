#include <cstdlib>
#include <boost/test/framework.hpp>
#include <test/tools/ossfuzz/abiV2FuzzerCommon.h>

using namespace dev::test::abiv2fuzzer;

SolidityExecutionFramework::SolidityExecutionFramework(langutil::EVMVersion _evmVersion)
{
	m_evmVersion = _evmVersion;
}

SolidityExecutionFramework::SolidityExecutionFramework()
{
	m_evmVersion = langutil::EVMVersion();
}