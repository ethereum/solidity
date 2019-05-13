#include <cstdlib>
#include <boost/test/framework.hpp>
#include "solidityutil.h"

using namespace solidityutil;

SolidityExecutionFramework::SolidityExecutionFramework(langutil::EVMVersion _evmVersion)
{
	m_evmVersion = _evmVersion;
}

SolidityExecutionFramework::SolidityExecutionFramework()
{
	m_evmVersion = langutil::EVMVersion();
}