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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity exception hierarchy.
 */

#pragma once

#include <libsolutil/Exceptions.h>
#include <libsolutil/Assertions.h>
#include <libsolutil/CommonData.h>
#include <liblangutil/SourceLocation.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/facilities/overload.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace solidity::langutil
{
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;

struct CompilerError: virtual util::Exception {};
struct StackTooDeepError: virtual CompilerError {};
struct InternalCompilerError: virtual util::Exception {};
struct FatalError: virtual util::Exception {};
struct UnimplementedFeatureError: virtual util::Exception {};
struct InvalidAstError: virtual util::Exception {};


/// Assertion that throws an InternalCompilerError containing the given description if it is not met.
#if !BOOST_PP_VARIADICS_MSVC
#define solAssert(...) BOOST_PP_OVERLOAD(solAssert_,__VA_ARGS__)(__VA_ARGS__)
#else
#define solAssert(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(solAssert_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define solAssert_1(CONDITION) \
	solAssert_2((CONDITION), "")

#define solAssert_2(CONDITION, DESCRIPTION) \
	assertThrowWithDefaultDescription( \
		(CONDITION), \
		::solidity::langutil::InternalCompilerError, \
		(DESCRIPTION), \
		"Solidity assertion failed" \
	)


/// Assertion that throws an UnimplementedFeatureError containing the given description if it is not met.
#if !BOOST_PP_VARIADICS_MSVC
#define solUnimplementedAssert(...) BOOST_PP_OVERLOAD(solUnimplementedAssert_,__VA_ARGS__)(__VA_ARGS__)
#else
#define solUnimplementedAssert(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(solUnimplementedAssert_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define solUnimplementedAssert_1(CONDITION) \
	solUnimplementedAssert_2((CONDITION), "")

#define solUnimplementedAssert_2(CONDITION, DESCRIPTION) \
	assertThrowWithDefaultDescription( \
		(CONDITION), \
		::solidity::langutil::UnimplementedFeatureError, \
		(DESCRIPTION), \
		"Unimplemented feature" \
	)


/// Helper that unconditionally reports an unimplemented feature.
#define solUnimplemented(DESCRIPTION) \
	solUnimplementedAssert(false, DESCRIPTION)


/// Assertion that throws an InvalidAstError containing the given description if it is not met.
#if !BOOST_PP_VARIADICS_MSVC
#define astAssert(...) BOOST_PP_OVERLOAD(astAssert_,__VA_ARGS__)(__VA_ARGS__)
#else
#define astAssert(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(astAssert_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define astAssert_1(CONDITION) \
	astAssert_2(CONDITION, "")

#define astAssert_2(CONDITION, DESCRIPTION) \
	assertThrowWithDefaultDescription( \
		(CONDITION), \
		::solidity::langutil::InvalidAstError, \
		(DESCRIPTION), \
		"AST assertion failed" \
	)


using errorSourceLocationInfo = std::pair<std::string, SourceLocation>;

class SecondarySourceLocation
{
public:
	SecondarySourceLocation& append(std::string const& _errMsg, SourceLocation const& _sourceLocation)
	{
		infos.emplace_back(_errMsg, _sourceLocation);
		return *this;
	}
	SecondarySourceLocation& append(SecondarySourceLocation&& _other)
	{
		infos += std::move(_other.infos);
		return *this;
	}

	/// Limits the number of secondary source locations to 32 and appends a notice to the
	/// error message.
	void limitSize(std::string& _message)
	{
		size_t occurrences = infos.size();
		if (occurrences > 32)
		{
			infos.resize(32);
			_message += " Truncated from " + std::to_string(occurrences) + " to the first 32 occurrences.";
		}
	}

	std::vector<errorSourceLocationInfo> infos;
};

using errinfo_sourceLocation = boost::error_info<struct tag_sourceLocation, SourceLocation>;
using errinfo_secondarySourceLocation = boost::error_info<struct tag_secondarySourceLocation, SecondarySourceLocation>;

/**
 * Unique identifiers are used to tag and track individual error cases.
 * They are passed as the first parameter of error reporting functions.
 * Suffix _error helps to find them in the sources.
 * The struct ErrorId prevents incidental calls like typeError(3141) instead of typeError(3141_error).
 * To create a new ID, one can add 0000_error and then run "python ./scripts/error_codes.py --fix"
 * from the root of the repo.
 */
struct ErrorId
{
	unsigned long long error = 0;
	bool operator==(ErrorId const& _rhs) const { return error == _rhs.error; }
	bool operator!=(ErrorId const& _rhs) const { return !(*this == _rhs); }
	bool operator<(ErrorId const& _rhs) const { return error < _rhs.error; }
};
constexpr ErrorId operator"" _error(unsigned long long _error) { return ErrorId{ _error }; }

class Error: virtual public util::Exception
{
public:
	enum class Type
	{
		Info,
		Warning,
		CodeGenerationError,
		DeclarationError,
		DocstringParsingError,
		ParserError,
		TypeError,
		SyntaxError,
		IOError,
		FatalError,
		JSONError,
		InternalCompilerError,
		CompilerError,
		Exception,
		UnimplementedFeatureError,
		YulException,
		SMTLogicException,
	};

	enum class Severity
	{
		// NOTE: We rely on these being ordered from least to most severe.
		Info,
		Warning,
		Error,
	};

	Error(
		ErrorId _errorId,
		Type _type,
		std::string const& _description,
		SourceLocation const& _location = SourceLocation(),
		SecondarySourceLocation const& _secondaryLocation = SecondarySourceLocation()
	);

	ErrorId errorId() const { return m_errorId; }
	Type type() const { return m_type; }
	Severity severity() const { return errorSeverity(m_type); }

	SourceLocation const* sourceLocation() const noexcept;
	SecondarySourceLocation const* secondarySourceLocation() const noexcept;

	/// helper functions
	static Error const* containsErrorOfType(ErrorList const& _list, Error::Type _type)
	{
		for (auto e: _list)
			if (e->type() == _type)
				return e.get();
		return nullptr;
	}

	static constexpr Severity errorSeverity(Type _type)
	{
		switch (_type)
		{
			case Type::Info: return Severity::Info;
			case Type::Warning: return Severity::Warning;
			default: return Severity::Error;
		}
	}

	static constexpr Severity errorSeverityOrType(std::variant<Error::Type, Error::Severity> _typeOrSeverity)
	{
		if (std::holds_alternative<Error::Type>(_typeOrSeverity))
			return errorSeverity(std::get<Error::Type>(_typeOrSeverity));
		return std::get<Error::Severity>(_typeOrSeverity);
	}

	static bool isError(Severity _severity)
	{
		return _severity == Severity::Error;
	}

	static bool isError(Type _type)
	{
		return isError(errorSeverity(_type));
	}

	static bool containsErrors(ErrorList const& _list)
	{
		for (auto e: _list)
			if (isError(e->type()))
				return true;
		return false;
	}

	static std::string formatErrorSeverity(Severity _severity)
	{
		switch (_severity)
		{
		case Severity::Info: return "Info";
		case Severity::Warning: return "Warning";
		case Severity::Error: return "Error";
		}
		util::unreachable();
	}

	static std::string formatErrorType(Type _type)
	{
		return m_errorTypeNames.at(_type);
	}

	static std::optional<Type> parseErrorType(std::string _name)
	{
		static std::map<std::string, Error::Type> const m_errorTypesByName = util::invertMap(m_errorTypeNames);

		if (m_errorTypesByName.count(_name) == 0)
			return std::nullopt;

		return m_errorTypesByName.at(_name);
	}

	static std::string formatTypeOrSeverity(std::variant<Error::Type, Error::Severity> _typeOrSeverity)
	{
		if (std::holds_alternative<Error::Type>(_typeOrSeverity))
			return formatErrorType(std::get<Error::Type>(_typeOrSeverity));
		return formatErrorSeverity(std::get<Error::Severity>(_typeOrSeverity));
	}

	static std::string formatErrorSeverityLowercase(Severity _severity)
	{
		std::string severityValue = formatErrorSeverity(_severity);
		boost::algorithm::to_lower(severityValue);
		return severityValue;
	}

private:
	ErrorId m_errorId;
	Type m_type;

	static std::map<Type, std::string> const m_errorTypeNames;
};

}
