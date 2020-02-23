#pragma once

#include <map>
#include <string>
#include <vector>

namespace solidity::test
{
class EndToEndCppTestFile
{
  public:
	EndToEndCppTestFile(std::string const &_sourceFile) : m_sourceFile(_sourceFile) {}

	void analyse();

	const std::map<std::string, std::vector<std::string>> &highlevelComments() { return m_highlevelComments; }

	const std::map<std::string, std::vector<std::pair<std::string, std::string>>> &expectationComments()
	{
		return m_expectationComments;
	}

  private:
	std::string m_sourceFile;
	std::map<std::string, std::vector<std::string>> m_highlevelComments;
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> m_expectationComments;
};

} // namespace solidity::test
