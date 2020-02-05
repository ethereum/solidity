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
#include <tools/solidityUpgrade/UpgradeChange.h>

#include <liblangutil/SourceReferenceExtractor.h>
#include <liblangutil/SourceReferenceFormatterHuman.h>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::tools;

void UpgradeChange::apply()
{
	m_source.replace(m_location.start, m_location.end - m_location.start, m_patch);
}

void UpgradeChange::log(bool const _shorten) const
{
	stringstream os;
	SourceReferenceFormatterHuman formatter{os, true};

	string start = to_string(m_location.start);
	string end = to_string(m_location.end);

	auto color = m_level == Level::Unsafe ? formatting::MAGENTA : formatting::CYAN;
	auto level = m_level == Level::Unsafe ? "unsafe" : "safe";

	os << endl;
	AnsiColorized(os, true, {formatting::BOLD, color}) << "Upgrade change (" << level << ")" << endl;
	os << "=======================" << endl;
	formatter.printSourceLocation(SourceReferenceExtractor::extract(&m_location));
	os << endl;

	LineColumn lineEnd = m_location.source->translatePositionToLineColumn(m_location.end);
	int const leftpad = static_cast<int>(log10(max(lineEnd.line, 1))) + 2;

	stringstream output;
	output << (_shorten ? shortenSource(m_patch) : m_patch);

	string line;
	while (getline(output, line))
	{
		os << string(leftpad, ' ');
		AnsiColorized(os, true, {formatting::BOLD, formatting::BLUE}) << "| ";
		AnsiColorized(os, true, {}) << line << endl;
	}
	cout << os.str();
	cout << endl;
}

string UpgradeChange::shortenSource(string const& _source)
{
	size_t constexpr maxSourceLength = 1000;
	if (_source.size() > maxSourceLength)
		return _source.substr(0, min(_source.size(), maxSourceLength)) + "...";
	return _source;
}
