// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Formatting functions for errors referencing positions and locations in the source.
 */

#pragma once

#include <ostream>
#include <sstream>
#include <functional>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceExtractor.h>

namespace solidity::util
{
struct Exception; // forward
}

namespace solidity::langutil
{
struct SourceLocation;
class Scanner;

class SourceReferenceFormatter
{
public:
	explicit SourceReferenceFormatter(std::ostream& _stream):
		m_stream(_stream)
	{}

	virtual ~SourceReferenceFormatter() = default;

	/// Prints source location if it is given.
	virtual void printSourceLocation(SourceReference const& _ref);
	virtual void printExceptionInformation(SourceReferenceExtractor::Message const& _msg);

	virtual void printSourceLocation(SourceLocation const* _location);
	virtual void printExceptionInformation(util::Exception const& _exception, std::string const& _category);
	virtual void printErrorInformation(Error const& _error);

	static std::string formatErrorInformation(Error const& _error)
	{
		return formatExceptionInformation(
			_error,
			(_error.type() == Error::Type::Warning) ? "Warning" : "Error"
		);
	}

	static std::string formatExceptionInformation(
		util::Exception const& _exception,
		std::string const& _name
	)
	{
		std::ostringstream errorOutput;

		SourceReferenceFormatter formatter(errorOutput);
		formatter.printExceptionInformation(_exception, _name);
		return errorOutput.str();
	}

protected:
	/// Prints source name if location is given.
	void printSourceName(SourceReference const& _ref);

	std::ostream& m_stream;
};

}
