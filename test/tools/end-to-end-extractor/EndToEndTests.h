#pragma once

#include "ExtractorExecutionFramework.h"
#include "ExtractionTask.h"

namespace solidity::test
{

class End2EndExtractor : public solidity::test::ExtractorExecutionFramework
{
  public:
	End2EndExtractor();

	TestSuite &testsuite() { return m_tests; }

  private:
	TestSuite m_tests;
};

} // namespace solidity::test