#include "EndToEndCppTestFile.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace solidity::test
{
void EndToEndCppTestFile::analyse()
{
	std::ifstream file(m_sourceFile);
	if (file.is_open())
	{
		std::string line;
		bool insideTest = false;
		bool insideSource = false;
		bool topComments = true;
		std::string test_name;

		while (getline(file, line))
		{
			if (boost::starts_with(line, "BOOST_AUTO_TEST_CASE("))
			{
				test_name = boost::replace_all_copy(line, "BOOST_AUTO_TEST_CASE(", "");
				test_name = test_name.substr(0, test_name.length() - 1);
				insideTest = true;
				topComments = true;
				insideSource = false;
			}
			if (insideTest && boost::starts_with(line, "}"))
				insideTest = false;

			if (insideTest)
			{
				boost::algorithm::trim(line);
				if (line.find("char const*") != std::string::npos)
					insideSource = true;
				if (line.find(")\";") != std::string::npos)
					insideSource = false;
				if (line.find("compileAndRun") != std::string::npos)
					topComments = false;

				if (!insideSource)
				{
					if (boost::starts_with(line, "//"))
					{
						if (topComments)
						{
							m_highlevelComments[test_name].emplace_back(boost::algorithm::trim_copy(line.substr(2)));
						}
						else
						{
							std::stringstream stream;
							do
							{
								if (boost::starts_with(line, "//"))
									stream << boost::algorithm::trim_copy(line.substr(2));
								getline(file, line); // read next line
								boost::algorithm::trim(line);
							} while (boost::starts_with(line, "//"));
							m_expectationComments[test_name].emplace_back(stream.str(), line);
						}
					}
				}
			}
		}
		file.close();
	}
}

} // namespace solidity::test
