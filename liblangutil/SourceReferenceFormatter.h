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

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceExtractor.h>

#include <libsolutil/AnsiColorized.h>

#include <ostream>
#include <sstream>
#include <functional>

namespace solidity::langutil
{

class CharStream;
class CharStreamProvider;
struct SourceLocation;

class SourceReferenceFormatter
{
public:
	SourceReferenceFormatter(
		std::ostream& _stream,
		CharStreamProvider const& _charStreamProvider,
		bool _colored,
		bool _withErrorIds
	):
		m_stream(_stream), m_charStreamProvider(_charStreamProvider), m_colored(_colored), m_withErrorIds(_withErrorIds)
	{}

	/// Prints source location if it is given.
	void printSourceLocation(SourceReference const& _ref);
	void printExceptionInformation(SourceReferenceExtractor::Message const& _msg, bool _printFullType=false);
	void printExceptionInformation(util::Exception const& _exception, Error::Type _type, bool _printFullType=false);
	void printErrorInformation(langutil::ErrorList const& _errors);
	void printErrorInformation(Error const& _error);

	static std::string formatExceptionInformation(
		util::Exception const& _exception,
		Error::Type _type,
		CharStreamProvider const& _charStreamProvider,
		bool _colored = false,
		bool _withErrorIds = false,
		bool _printFullType = false
	)
	{
		std::ostringstream errorOutput;
		SourceReferenceFormatter formatter(errorOutput, _charStreamProvider, _colored, _withErrorIds);
		formatter.printExceptionInformation(_exception, _type, _printFullType);
		return errorOutput.str();
	}

	static std::string formatErrorInformation(
		Error const& _error,
		CharStreamProvider const& _charStreamProvider
	)
	{
		return formatExceptionInformation(
			_error,
			_error.type(),
			_charStreamProvider
		);
	}

	static std::string formatErrorInformation(Error const& _error, CharStream const& _charStream);

private:
	util::AnsiColorized normalColored() const;
	util::AnsiColorized frameColored() const;
	util::AnsiColorized errorColored(langutil::Error::Severity _severity) const;
	util::AnsiColorized messageColored() const;
	util::AnsiColorized secondaryColored() const;
	util::AnsiColorized highlightColored() const;
	util::AnsiColorized diagColored() const;

private:
	std::ostream& m_stream;
	CharStreamProvider const& m_charStreamProvider;
	bool m_colored;
	bool m_withErrorIds;
};

}
