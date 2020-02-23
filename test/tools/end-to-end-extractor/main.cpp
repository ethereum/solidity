#include "EndToEndTests.h"
#include <iostream>

int main()
{
	auto options = std::make_unique<solidity::test::CommonOptions>();
	solidity::test::CommonOptions::setSingleton(std::move(options));
	solidity::test::End2EndExtractor extractor;

	uint32_t extractable{0};
	uint32_t non_extractable{0};

	std::cout << SOLIDITY_ROOT << std::endl;
	std::cout << "- analysing test-cases..." << std::endl;
	extractor.analyze();
	for (auto &task : extractor.testsuite())
	{
		if (task.second.extractable())
			++extractable;
		else
			++non_extractable;
	}
	std::cout << "- analysing test-cases... done" << std::endl;

	std::cout << "  "<< extractable << " / " << extractable + non_extractable << ", " << non_extractable
	          << " test-cases are not extractable" << std::endl;

	std::cout << "- extracting test-cases..." << std::endl;
	for (auto &task : extractor.testsuite())
		if (task.second.extractable())
			task.second.extract();
	std::cout << "- extracting test-cases... done" << std::endl;



	return 0;
}
