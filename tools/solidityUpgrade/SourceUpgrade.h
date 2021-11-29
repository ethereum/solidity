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

#include <tools/solidityUpgrade/UpgradeChange.h>
#include <tools/solidityUpgrade/Upgrade050.h>
#include <tools/solidityUpgrade/Upgrade060.h>
#include <tools/solidityUpgrade/Upgrade070.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/DebugSettings.h>
#include <liblangutil/EVMVersion.h>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

#include <memory>

namespace solidity::tools
{

/**
 * The Solidity source upgrade tool. It supplies a command line interface
 * and connects this to a compiler stack that the upgrade logic is facilitated
 * with.
 */
class SourceUpgrade
{
public:
	/// Parse command line arguments and return false in case of a failure.
	bool parseArguments(int _argc, char** _argv);
	/// Prints additional information on the upgrade tool.
	void printPrologue();
	/// Parse / compile files and runs upgrade analysis on them.
	bool processInput();

private:
	/// All available upgrade modules
	enum class Module
	{
		ConstructorKeyword,
		VisibilitySpecifier,
		AbstractContract,
		OverridingFunction,
		VirtualFunction,
		DotSyntax,
		NowKeyword,
		ConstrutorVisibility
	};

	/// Upgrade suite that hosts all available modules.
	class Suite: public UpgradeSuite
	{
	public:
		void analyze(langutil::CharStreamProvider const& _charStreamProvider, frontend::SourceUnit const& _sourceUnit)
		{
			/// Solidity 0.5.0
			if (isActivated(Module::ConstructorKeyword))
				ConstructorKeyword{_charStreamProvider, m_changes}.analyze(_sourceUnit);
			if (isActivated(Module::VisibilitySpecifier))
				VisibilitySpecifier{_charStreamProvider, m_changes}.analyze(_sourceUnit);

			/// Solidity 0.6.0
			if (isActivated(Module::AbstractContract))
				AbstractContract{_charStreamProvider, m_changes}.analyze(_sourceUnit);
			if (isActivated(Module::OverridingFunction))
				OverridingFunction{_charStreamProvider, m_changes}.analyze(_sourceUnit);
			if (isActivated(Module::VirtualFunction))
				VirtualFunction{_charStreamProvider, m_changes}.analyze(_sourceUnit);

			/// Solidity 0.7.0
			if (isActivated(Module::DotSyntax))
				DotSyntax{_charStreamProvider, m_changes}.analyze(_sourceUnit);
			if (isActivated(Module::NowKeyword))
				NowKeyword{_charStreamProvider, m_changes}.analyze(_sourceUnit);
			if (isActivated(Module::ConstrutorVisibility))
				ConstructorVisibility{_charStreamProvider, m_changes}.analyze(_sourceUnit);
		}

		void activateModule(Module _module) { m_modules.insert(_module); }
		void deactivateModules() { m_modules.clear(); }

	private:
		bool isActivated(Module _module) const
		{
			return m_modules.find(_module) != m_modules.end();
		}

		/// All modules are activated by default. Clear them before activating
		/// single ones.
		std::set<Module> m_modules = {
			Module::ConstructorKeyword,
			Module::VisibilitySpecifier,
			Module::AbstractContract,
			Module::OverridingFunction,
			Module::VirtualFunction,
			Module::DotSyntax,
			Module::NowKeyword,
			Module::ConstrutorVisibility
		};
	};

	/// Parses the current sources and runs analyses as well as compilation on
	/// them if parsing was successful.
	void tryCompile() const;
	/// Analyses and upgrades the sources given. The upgrade happens in a loop,
	/// applying one change at a time, which is run until no applicable changes
	/// are found any more. Only one change is done at a time and all sources
	/// are being compiled again after each change.
	void runUpgrade();
	/// Runs upgrade analysis on source and applies upgrades changes to it.
	/// Returns `true` if there're still changes that can be applied,
	/// `false` otherwise.
	bool analyzeAndUpgrade(
		std::pair<std::string, std::string> const& _sourceCode
	);

	/// Applies the change given to its source code. If no `--dry-run` was
	/// passed via the commandline, the upgraded source code is written back
	/// to its file.
	void applyChange(
		std::pair<std::string, std::string> const& _sourceCode,
		UpgradeChange& _change
	);

	/// Prints all errors (excluding warnings) the compiler currently reported.
	void printErrors() const;
	/// Prints error and upgrade overview at the end of each full run.
	void printStatistics() const;

	/// Reads all input files given and stores sources in the internal data
	/// structure. Reports errors if files cannot be found.
	bool readInputFiles();
	/// Writes source to file given.
	bool writeInputFile(std::string const& _path, std::string const& _source);
	/// Returns a file reader function that fills `m_sources`.
	frontend::ReadCallback::Callback fileReader();

	/// Resets the compiler stack and configures sources to compile.
	/// Also enables error recovery.
	void resetCompiler();
	/// Resets the compiler stack and configures sources to compile.
	/// Also enables error recovery. Passes read callback to the compiler stack.
	void resetCompiler(frontend::ReadCallback::Callback const& _callback);

	/// Compiler arguments variable map
	boost::program_options::variables_map m_args;
	/// Map of input files to source code strings
	std::map<std::string, std::string> m_sourceCodes;
	/// Solidity compiler stack
	std::unique_ptr<frontend::CompilerStack> m_compiler;
	/// List of allowed directories to read files from
	std::vector<boost::filesystem::path> m_allowedDirectories;
	/// Holds all upgrade modules and source upgrades.
	Suite m_suite;
};

}
