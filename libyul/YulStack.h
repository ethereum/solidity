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
 * Full assembly stack that can support Yul as input.
 */

#pragma once

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/EVMVersion.h>
#include <libsolutil/JSON.h>

#include <libyul/Object.h>
#include <libyul/ObjectOptimizer.h>
#include <libyul/ObjectParser.h>

#include <libsolidity/interface/OptimiserSettings.h>

#include <libevmasm/LinkerObject.h>

#include <memory>
#include <string>

namespace solidity::evmasm
{
class Assembly;
}

namespace solidity::langutil
{
class Scanner;
}

namespace solidity::yul
{
class AbstractAssembly;


struct MachineAssemblyObject
{
	std::shared_ptr<evmasm::LinkerObject> bytecode;
	std::shared_ptr<evmasm::Assembly> assembly;
	std::unique_ptr<std::string> sourceMappings;
};

/*
 * Full assembly stack that can support EVM-assembly and Yul as input and EVM as output.
 */
class YulStack: public langutil::CharStreamProvider
{
public:
	using Language = yul::Language;
	enum class Machine { EVM };
	enum State {
		Empty,
		Parsed,
		AnalysisSuccessful
	};

	YulStack():
		YulStack(
			langutil::EVMVersion{},
			std::nullopt,
			Language::Assembly,
			solidity::frontend::OptimiserSettings::none(),
			langutil::DebugInfoSelection::Default()
		)
	{}

	YulStack(
		langutil::EVMVersion _evmVersion,
		std::optional<uint8_t> _eofVersion,
		Language _language,
		solidity::frontend::OptimiserSettings _optimiserSettings,
		langutil::DebugInfoSelection const& _debugInfoSelection,
		std::shared_ptr<ObjectOptimizer> _objectOptimizer = nullptr
	):
		m_language(_language),
		m_evmVersion(_evmVersion),
		m_eofVersion(_eofVersion),
		m_optimiserSettings(std::move(_optimiserSettings)),
		m_debugInfoSelection(_debugInfoSelection),
		m_errorReporter(m_errors),
		m_objectOptimizer(_objectOptimizer ? std::move(_objectOptimizer) : std::make_shared<ObjectOptimizer>())
	{}

	/// @returns the char stream used during parsing
	langutil::CharStream const& charStream(std::string const& _sourceName) const override;

	/// Runs parsing and analysis steps, returns false if input cannot be assembled.
	/// Multiple calls overwrite the previous state.
	bool parseAndAnalyze(std::string const& _sourceName, std::string const& _source);

	/// Run the optimizer suite. Can only be used with Yul or strict assembly.
	/// If the settings (see constructor) disabled the optimizer, nothing is done here.
	void optimize();

	/// Run the assembly step (should only be called after parseAndAnalyze).
	MachineAssemblyObject assemble(Machine _machine);

	/// Run the assembly step (should only be called after parseAndAnalyze).
	/// In addition to the value returned by @a assemble, returns
	/// a second object that is the runtime code.
	/// Only available for EVM.
	std::pair<MachineAssemblyObject, MachineAssemblyObject>
	assembleWithDeployed(
		std::optional<std::string_view> _deployName = {}
	);

	/// Run the assembly step (should only be called after parseAndAnalyze).
	/// Similar to @a assemblyWithDeployed, but returns EVM assembly objects.
	/// Only available for EVM.
	std::pair<std::shared_ptr<evmasm::Assembly>, std::shared_ptr<evmasm::Assembly>>
	assembleEVMWithDeployed(
		std::optional<std::string_view> _deployName = {}
	);

	/// @returns the errors generated during parsing, analysis (and potentially assembly).
	langutil::ErrorList const& errors() const { return m_errors; }

	/// Pretty-print the input after having parsed it.
	std::string print(
		langutil::CharStreamProvider const* _soliditySourceProvider = nullptr
	) const;
	Json astJson() const;
	std::string astCoq() const;

	// return the JSON representation of the YuL CFG (experimental)
	Json cfgJson() const;

	/// Return the parsed and analyzed object.
	std::shared_ptr<Object> parserResult() const;

	langutil::DebugInfoSelection debugInfoSelection() const { return m_debugInfoSelection; }

private:
	bool parse(std::string const& _sourceName, std::string const& _source);
	bool analyzeParsed();
	bool analyzeParsed(yul::Object& _object);

	void compileEVM(yul::AbstractAssembly& _assembly, bool _optimize) const;

	/// Prints the Yul object stored internally and parses it again.
	/// This ensures that the debug info in the AST matches the source that printing would produce
	/// rather than the initial source.
	/// @warning Does not update the error list or the original source (@a m_charStream) to make
	/// it possible to report errors to the user even after the optimization has been performed.
	void reparse();

	void reportUnimplementedFeatureError(langutil::UnimplementedFeatureError const& _error);

	Language m_language = Language::Assembly;
	langutil::EVMVersion m_evmVersion;
	std::optional<uint8_t> m_eofVersion;
	solidity::frontend::OptimiserSettings m_optimiserSettings;
	langutil::DebugInfoSelection m_debugInfoSelection{};

	std::unique_ptr<langutil::CharStream> m_charStream;

	State m_stackState = Empty;
	std::shared_ptr<yul::Object> m_parserResult;
	langutil::ErrorList m_errors;
	langutil::ErrorReporter m_errorReporter;

	std::shared_ptr<ObjectOptimizer> m_objectOptimizer;
};

}
