#pragma once

#include "EndToEndCppTestFile.h"
#include "ExtractionTask.h"
#include "ExtractorExecutionFramework.h"

namespace solidity::test
{
class End2EndExtractor : public solidity::test::ExtractorExecutionFramework
{
  public:
	End2EndExtractor();

	TestSuite &testsuite() { return m_tests; }

	void analyze()
	{
		for (auto &task : m_tests)
			task.second.analyse();

		if (!m_cppSource)
			m_cppSource = std::make_unique<EndToEndCppTestFile>(std::string(SOLIDITY_ROOT)
			                                                    + "/test/libsolidity/SolidityEndToEndTest.cpp");
		m_cppSource->analyse();

		for (auto &comments : m_cppSource->highlevelComments())
			m_tests[comments.first].setHighlevelComments(comments.second);

		for (auto &comments : m_cppSource->expectationComments())
			m_tests[comments.first].setExpectationComments(comments.second);
	}

  private:
	TestSuite m_tests;
	std::unique_ptr<EndToEndCppTestFile> m_cppSource;
};

} // namespace solidity::test