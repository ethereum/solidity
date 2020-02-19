#include "ExtractionTask.h"

void ExtractionTask::extract()
{
	assert(m_sources.size() == 1);
	assert(!m_expectations.empty());

	std::ofstream stream("/tmp/e2ev2/" + m_name + ".sol");
	stream << *m_sources.begin();
	stream << std::endl;

	if (m_alsoViaYul)
	{
		stream << "// ====" << std::endl;
		stream << "// optimize-yul: false" << std::endl;
	}
	stream << "// ----" << std::endl;
	for (auto const &expectation : m_expectations)
		stream << expectation << std::endl;
	stream << std::endl;
}
