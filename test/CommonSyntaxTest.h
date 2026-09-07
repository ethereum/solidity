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

#pragma once

#include <test/TestCase.h>
#include <liblangutil/Exceptions.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/formal/ModelCheckerSettings.h>
#include <libsolutil/AnsiColorized.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

using solidity::frontend::CompilerStack;
using solidity::frontend::ModelCheckerSettings;
using solidity::frontend::OptimiserSettings;

namespace solidity::test
{

struct SyntaxTestError
{
	langutil::Error::Type type;
	std::optional<langutil::ErrorId> errorId;
	std::string message;
	std::string sourceName;
	int locationStart = -1;
	int locationEnd = -1;
	bool operator==(SyntaxTestError const& _rhs) const
	{
		return type == _rhs.type &&
			errorId == _rhs.errorId &&
			message == _rhs.message &&
			sourceName == _rhs.sourceName &&
			locationStart == _rhs.locationStart &&
			locationEnd == _rhs.locationEnd;
	}
};

/**
 * Input that the compiler is called with.
 */
struct CompilerInput
{
	/// Default-constructable, non-copyable.
	CompilerInput() = default;
	CompilerInput(CompilerInput const&) = delete;
	CompilerInput& operator=(CompilerInput const&) = delete;
	CompilerInput(CompilerInput&&) = default;
	CompilerInput& operator=(CompilerInput&&) = default;

	std::map<std::string, std::string> sources;
	langutil::EVMVersion evmVersion;
	bool experimental = false;
	bool viaIR = false;
	bool viaSSACFG = false;
	OptimiserSettings optimiserSettings;
	CompilerStack::MetadataFormat metadataFormat = CompilerStack::MetadataFormat::NoMetadata;
	CompilerStack::MetadataHash metadataHash = CompilerStack::MetadataHash::None;
	ModelCheckerSettings modelCheckerSettings;
};

class CommonSyntaxTest: public frontend::test::EVMVersionRestrictedTestCase
{
public:
	CommonSyntaxTest(std::string const& _filename, langutil::EVMVersion _evmVersion);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

	void printSource(std::ostream& _stream, std::string const &_linePrefix = "", bool _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override
	{
		printObtainedResult(_stream, _linePrefix, false);
	}

	static std::string errorMessage(util::Exception const& _e);
protected:
	/// Should be implemented by those derived test cases that want to allow extra expectations
	/// after the error/warning expectations. The default implementation does not allow them and
	/// fails instead.
	/// @param _stream Input stream positioned at the beginning of the extra expectations.
	virtual void parseCustomExpectations(std::istream& _stream);

	virtual void parseAndAnalyze() = 0;

	/// Should return true if obtained values match expectations.
	/// The default implementation only compares the error list. Derived classes that support
	/// custom expectations should override this to include them in the comparison.
	virtual bool expectationsMatch();

	virtual void printExpectedResult(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const;
	virtual void printObtainedResult(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const;

	static void printErrorList(
		std::ostream& _stream,
		std::vector<SyntaxTestError> const& _errors,
		std::string const& _linePrefix,
		bool _formatted = false
	);

	TestResult conclude(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false);
	void printExpectationAndError(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false);

	static std::vector<SyntaxTestError> parseExpectations(std::istream& _stream);

	std::vector<SyntaxTestError> m_expectations;
	std::vector<SyntaxTestError> m_errorList;
	CompilerInput m_compilerInput;
};

}
