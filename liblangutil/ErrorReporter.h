// SPDX-License-Identifier: GPL-3.0
/**
 * @author Rhett <roadriverrail@gmail.com>
 * @date 2017
 * Error reporting helper class.
 */

#pragma once

#include <libsolutil/CommonData.h>
#include <libsolutil/Exceptions.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceLocation.h>
#include <libsolutil/StringUtils.h>

#include <boost/range/adaptor/filtered.hpp>

namespace solidity::langutil
{

class ErrorReporter
{
public:

	explicit ErrorReporter(ErrorList& _errors):
		m_errorList(_errors) { }

	ErrorReporter(ErrorReporter const& _errorReporter) noexcept:
		m_errorList(_errorReporter.m_errorList) { }

	ErrorReporter& operator=(ErrorReporter const& _errorReporter);

	void append(ErrorList const& _errorList)
	{
		m_errorList += _errorList;
	}

	void warning(ErrorId _error, std::string const& _description);

	void warning(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void warning(
		ErrorId _error,
		SourceLocation const& _location,
		std::string const& _description,
		SecondarySourceLocation const& _secondaryLocation
	);

	void error(
		ErrorId _error,
		Error::Type _type,
		SourceLocation const& _location,
		std::string const& _description
	);

	void declarationError(
		ErrorId _error,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description
	);

	void declarationError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void fatalDeclarationError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void parserError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void fatalParserError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void syntaxError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void typeError(
		ErrorId _error,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation = SecondarySourceLocation(),
		std::string const& _description = std::string()
	);

	void typeError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	template <typename... Strings>
	void typeErrorConcatenateDescriptions(ErrorId _error, SourceLocation const& _location, Strings const&... _descriptions)
	{
		std::initializer_list<std::string> const descs = { _descriptions... };
		solAssert(descs.size() > 0, "Need error descriptions!");

		auto filterEmpty = boost::adaptors::filtered([](std::string const& _s) { return !_s.empty(); });

		std::string errorStr = util::joinHumanReadable(descs | filterEmpty, " ");

		error(_error, Error::Type::TypeError, _location, errorStr);
	}

	void fatalTypeError(ErrorId _error, SourceLocation const& _location, std::string const& _description);
	void fatalTypeError(ErrorId _error, SourceLocation const& _location, SecondarySourceLocation const& _secondLocation, std::string const& _description);

	void docstringParsingError(ErrorId _error, std::string const& _description);
	void docstringParsingError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	ErrorList const& errors() const;

	void clear();

	/// @returns true iff there is any error (ignores warnings).
	bool hasErrors() const
	{
		return m_errorCount > 0;
	}

	/// @returns the number of errors (ignores warnings).
	unsigned errorCount() const
	{
		return m_errorCount;
	}

	// @returns true if the maximum error count has been reached.
	bool hasExcessiveErrors() const;

	class ErrorWatcher
	{
	public:
		ErrorWatcher(ErrorReporter const& _errorReporter):
			m_errorReporter(_errorReporter),
			m_initialErrorCount(_errorReporter.errorCount())
		{}
		bool ok() const
		{
			solAssert(m_initialErrorCount <= m_errorReporter.errorCount(), "Unexpected error count.");
			return m_initialErrorCount == m_errorReporter.errorCount();
		}
	private:
		ErrorReporter const& m_errorReporter;
		unsigned const m_initialErrorCount;
	};

	ErrorWatcher errorWatcher() const
	{
		return ErrorWatcher(*this);
	}

private:
	void error(
		ErrorId _error,
		Error::Type _type,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description = std::string());

	void fatalError(
		ErrorId _error,
		Error::Type _type,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description = std::string());

	void fatalError(
		ErrorId _error,
		Error::Type _type,
		SourceLocation const& _location = SourceLocation(),
		std::string const& _description = std::string());

	// @returns true if error shouldn't be stored
	bool checkForExcessiveErrors(Error::Type _type);

	ErrorList& m_errorList;

	unsigned m_errorCount = 0;
	unsigned m_warningCount = 0;

	unsigned const c_maxWarningsAllowed = 256;
	unsigned const c_maxErrorsAllowed = 256;
};

}
