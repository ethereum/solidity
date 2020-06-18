/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <test/tools/ossfuzz/SolImportProtoConverter.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace solidity::test::solimportprotofuzzer;
using namespace std;

map<string, string> ProtoConverter::sourceCodeMap(Test const& _input)
{
	// Setup random number generator
	m_randomGen = make_unique<SolRandomNumGenerator>(_input.seed());
	m_numSources = _input.source_size();
	unsigned sourceIndex = 0;
	for (auto const& source: _input.source())
		m_sourceMap["i" + to_string(sourceIndex++) + ".sol"] = visit(source);
	return m_sourceMap;
}

string ProtoConverter::visit(Source const& _source)
{
	// A source file may import itself and any of the
	// other source files.
	unsigned numImports = _source.num_imports() % (m_numSources + 1);
	string sourceCode = {};
	set<unsigned> importSet;
	for (unsigned i = 0; i < numImports; i++)
	{
		unsigned sourceIndex = randomNum() % m_numSources;
		// Avoid duplicate imports
		if (importSet.count(sourceIndex))
			continue;
		else
			importSet.insert(sourceIndex);
		string sourceName = "i" + to_string(sourceIndex) + ".sol";
		string importName = "I" + to_string(sourceIndex);
		sourceCode += "import \"" + sourceName + "\" as " + importName + ";\n";
	}
	sourceCode += _source.code();
	boost::range::remove_erase_if(sourceCode, [=](char c) -> bool {
	  return !(std::isprint(c) || c == '\n');
	});
	return sourceCode;
}