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

#include <libevmasm/CodeSizeLimits.h>

#include <liblangutil/ErrorReporter.h>

#include <limits>

using namespace std::string_literals;
using namespace solidity::langutil;

namespace solidity::evmasm
{

namespace
{

/// Runtime code size limit (in bytes) introduced by EIP-170 in the Spurious Dragon release.
constexpr size_t RUNTIME_CODE_SIZE_LIMIT_SPURIOUS_DRAGON = 24576;

/// Initcode size limit (in bytes) introduced by EIP-3860 in the Shanghai release.
constexpr size_t CREATION_CODE_SIZE_LIMIT_SHANGHAI = 49152;

struct CodeSizeLimit
{
	std::string forkName = {};
	size_t limit = std::numeric_limits<size_t>::max();
	ErrorId errorID = {};
};

CodeSizeLimit creationCodeSizeLimit(EVMVersion _evmVersion)
{
	if (_evmVersion >= EVMVersion::shanghai())
		return {
			"Shanghai",
			CREATION_CODE_SIZE_LIMIT_SHANGHAI,
			3860_error,
		};
	else
		return {};
}

CodeSizeLimit runtimeCodeSizeLimit(EVMVersion _evmVersion)
{
	if (_evmVersion >= EVMVersion::spuriousDragon())
		return {
			"Spurious Dragon",
			RUNTIME_CODE_SIZE_LIMIT_SPURIOUS_DRAGON,
			5574_error,
		};
	else
		return {};
}

void reportCodeSizeLimitWarningIfExceeded(
	ErrorReporter& _errorReporter,
	CodeSizeLimit const& _codeSizeLimit,
	size_t _codeSize,
	std::string const& _subject,
	SourceLocation const& _location
)
{
	if (_codeSize > _codeSizeLimit.limit)
		_errorReporter.warning(
			_codeSizeLimit.errorID,
			_location,
			_subject + " size is "s +
			std::to_string(_codeSize) +
			" bytes and exceeds " + std::to_string(_codeSizeLimit.limit) +
			" bytes (a limit introduced in " + _codeSizeLimit.forkName + "). "
			"This contract may not be deployable on Mainnet. "
			"Consider enabling the optimizer (with a low \"runs\" value!), "
			"turning off revert strings, or using libraries."
		);
}

}

void checkCodeSizeLimits(
	ErrorReporter& _errorReporter,
	EVMVersion _evmVersion,
	SourceLocation const& _location,
	std::string const& _codeSizeWarningSubject,
	size_t _creationCodeSize,
	size_t _runtimeCodeSize
)
{
	reportCodeSizeLimitWarningIfExceeded(
		_errorReporter,
		runtimeCodeSizeLimit(_evmVersion),
		_runtimeCodeSize,
		_codeSizeWarningSubject + " code",
		_location
	);

	reportCodeSizeLimitWarningIfExceeded(
		_errorReporter,
		creationCodeSizeLimit(_evmVersion),
		_creationCodeSize,
		_codeSizeWarningSubject + " initcode",
		_location
	);
}

}
