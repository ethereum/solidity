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
// SPDX-License-Identifier: GPL-3.0
/**
 * Formatting functions for errors referencing positions and locations in the source.
 */

#pragma once

#include <liblangutil/SourceReferenceExtractor.h>
#include <liblangutil/SourceReferenceFormatter.h> // SourceReferenceFormatterBase

#include <libsolutil/AnsiColorized.h>

#include <ostream>
#include <sstream>
#include <functional>

namespace solidity::langutil
{

class SourceReferenceFormatterHuman: public SourceReferenceFormatter
{
public:
	SourceReferenceFormatterHuman(std::ostream& _stream, bool _colored, bool _withErrorIds):
		SourceReferenceFormatter{_stream}, m_colored{_colored}, m_withErrorIds(_withErrorIds)
	{}

	void printSourceLocation(SourceReference const& _ref) override;
	void printExceptionInformation(SourceReferenceExtractor::Message const& _msg) override;
	using SourceReferenceFormatter::printExceptionInformation;

	static std::string formatExceptionInformation(
		util::Exception const& _exception,
		std::string const& _name,
		bool _colored = false,
		bool _withErrorIds = false
	)
	{
		std::ostringstream errorOutput;

		SourceReferenceFormatterHuman formatter(errorOutput, _colored, _withErrorIds);
		formatter.printExceptionInformation(_exception, _name);
		return errorOutput.str();
	}

private:
	util::AnsiColorized normalColored() const;
	util::AnsiColorized frameColored() const;
	util::AnsiColorized errorColored() const;
	util::AnsiColorized messageColored() const;
	util::AnsiColorized secondaryColored() const;
	util::AnsiColorized highlightColored() const;
	util::AnsiColorized diagColored() const;

private:
	bool m_colored;
	bool m_withErrorIds;
};

}
