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
 * @author Alex Beregszaszi
 * @date 2016
 * Standard JSON compiler interface.
 */

#pragma once

#include <libsolidity/interface/CompilerStack.h>
#include <libsolutil/JSON.h>

#include <liblangutil/DebugInfoSelection.h>

#include <optional>
#include <utility>
#include <variant>

namespace solidity::frontend
{

/**
 * Standard JSON compiler interface, which expects a JSON input and returns a JSON output.
 * See docs/using-the-compiler#compiler-input-and-output-json-description.
 */
class StandardCompiler
{
public:
	/// Noncopyable.
	StandardCompiler(StandardCompiler const&) = delete;
	StandardCompiler& operator=(StandardCompiler const&) = delete;

	/// Creates a new StandardCompiler.
	/// @param _readFile callback used to read files for import statements. Must return
	/// and must not emit exceptions.
	explicit StandardCompiler(ReadCallback::Callback _readFile = ReadCallback::Callback(),
		util::JsonFormat const& _format = {}):
		m_readFile(std::move(_readFile)),
		m_jsonPrintingFormat(std::move(_format))
	{
	}

	/// Sets all input parameters according to @a _input which conforms to the standardized input
	/// format, performs compilation and returns a standardized output.
	Json::Value compile(Json::Value const& _input) noexcept;
	/// Parses input as JSON and peforms the above processing steps, returning a serialized JSON
	/// output. Parsing errors are returned as regular errors.
	std::string compile(std::string const& _input) noexcept;

	static Json::Value formatFunctionDebugData(
		std::map<std::string, evmasm::LinkerObject::FunctionDebugData> const& _debugInfo
	);

private:
	struct InputsAndSettings
	{
		std::string language;
		Json::Value errors;
		bool parserErrorRecovery = false;
		CompilerStack::State stopAfter = CompilerStack::State::CompilationSuccessful;
		std::map<std::string, std::string> sources;
		std::map<util::h256, std::string> smtLib2Responses;
		langutil::EVMVersion evmVersion;
		std::vector<ImportRemapper::Remapping> remappings;
		RevertStrings revertStrings = RevertStrings::Default;
		OptimiserSettings optimiserSettings = OptimiserSettings::minimal();
		std::optional<langutil::DebugInfoSelection> debugInfoSelection;
		std::map<std::string, util::h160> libraries;
		bool metadataLiteralSources = false;
		CompilerStack::MetadataFormat metadataFormat = CompilerStack::defaultMetadataFormat();
		CompilerStack::MetadataHash metadataHash = CompilerStack::MetadataHash::IPFS;
		Json::Value outputSelection;
		ModelCheckerSettings modelCheckerSettings = ModelCheckerSettings{};
		bool viaIR = false;
	};

	/// Parses the input json (and potentially invokes the read callback) and either returns
	/// it in condensed form or an error as a json object.
	std::variant<InputsAndSettings, Json::Value> parseInput(Json::Value const& _input);

	Json::Value compileSolidity(InputsAndSettings _inputsAndSettings);
	Json::Value compileYul(InputsAndSettings _inputsAndSettings);

	ReadCallback::Callback m_readFile;

	util::JsonFormat m_jsonPrintingFormat;
};

}
