#pragma once

#include <imgui.h>

#include "solc/CommandLineParser.h"

#include <boost/algorithm/string/case_conv.hpp>


namespace solidity::ui
{

class CommandLineOptions
{
public:
	explicit CommandLineOptions(solidity::frontend::CommandLineOptions& _options): m_options(_options) {}

	void render(
		std::function<bool(const char*, bool)> _begin = ImGui::BeginMenu,
		std::function<void(void)> _end = ImGui::EndMenu)
	{
		//			struct
		//			{
		//				InputMode mode = InputMode::Compiler;
		//				std::set<boost::filesystem::path> paths;
		//				std::vector<ImportRemapper::Remapping> remappings;
		//				bool addStdin = false;
		//				boost::filesystem::path basePath = "";
		//				std::vector<boost::filesystem::path> includePaths;
		//				FileReader::FileSystemPathSet allowedDirectories;
		//				bool ignoreMissingFiles = false;
		//				bool errorRecovery = false;
		//			} input;
		if (_begin("Input", true))
		{
			std::string selection;
			static const char* items[] = {
				"License",
				"Version",
				"Compiler",
				"Compiler With AST Import",
				"Standard JSON",
				"Linker",
				"Assembler",
				"Language Server",
			};
			ImGui::Combo("Input Mode", (int*) (&m_options.input.mode), items, IM_ARRAYSIZE(items), 4);
			ImGui::Checkbox("Add STDIN", &m_options.input.addStdin);
			ImGui::SameLine();
			ImGui::Checkbox("Ignore Missing Files", &m_options.input.ignoreMissingFiles);
			ImGui::SameLine();
			ImGui::Checkbox("Error Recovery", &m_options.input.errorRecovery);
			char basePath[1024];
			memset(basePath, 0, sizeof(basePath));
			memcpy(basePath, m_options.input.basePath.string().c_str(), m_options.input.basePath.string().length());
			if (ImGui::InputText("Base Path", basePath, sizeof(basePath)))
			{
				m_options.input.basePath = std::string(basePath);
			}
			_end();
		}
		//			struct
		//			{
		//				boost::filesystem::path dir;
		//				bool overwriteFiles = false;
		//				langutil::EVMVersion evmVersion;
		//				bool viaIR = false;
		//				RevertStrings revertStrings = RevertStrings::Default;
		//				std::optional<langutil::DebugInfoSelection> debugInfoSelection;
		//				CompilerStack::State stopAfter = CompilerStack::State::CompilationSuccessful;
		//			} output;
		if (_begin("Output", true))
		{
			char directory[1024];
			memset(directory, 0, sizeof(directory));
			memcpy(directory, m_options.output.dir.string().c_str(), m_options.output.dir.string().length());
			if (ImGui::InputText("Directory", directory, sizeof(directory)))
				m_options.output.dir = std::string(directory);
			static std::vector<solidity::langutil::EVMVersion> evm_versions{
				solidity::langutil::EVMVersion::homestead(),
				solidity::langutil::EVMVersion::tangerineWhistle(),
				solidity::langutil::EVMVersion::spuriousDragon(),
				solidity::langutil::EVMVersion::byzantium(),
				solidity::langutil::EVMVersion::constantinople(),
				solidity::langutil::EVMVersion::petersburg(),
				solidity::langutil::EVMVersion::istanbul(),
				solidity::langutil::EVMVersion::berlin(),
				solidity::langutil::EVMVersion::london(),
			};
			static const char* evm_versions_text[] = {
				"Homestead",
				"Tangerine Whistle",
				"SpuriousDragon",
				"Byzantium",
				"Constantinople",
				"Petersburg",
				"Istanbul",
				"Berlin",
				"London",
			};
			int evm_version_index = -1;
			int evm_version_current_index = 0;
			for (auto const& version: evm_versions)
			{
				if (version.name() == m_options.output.evmVersion.name())
					evm_version_index = evm_version_current_index;
				++evm_version_current_index;
			}
			if (ImGui::Combo("EVM Version", &evm_version_index, evm_versions_text, IM_ARRAYSIZE(evm_versions_text), 9))
				m_options.output.evmVersion = solidity::langutil::EVMVersion::fromString(
												  evm_versions[static_cast<unsigned int>(evm_version_index)].name())
												  .value();
			static const char* revert_strings_items[] = {"Default", "Strip", "Debug", "Verbose Debug"};
			int revert_strings_index = (int) m_options.output.revertStrings;
			if (ImGui::Combo(
					"Revert Strings",
					&revert_strings_index,
					revert_strings_items,
					IM_ARRAYSIZE(revert_strings_items),
					4))
				m_options.output.revertStrings = static_cast<solidity::frontend::RevertStrings>(revert_strings_index);
			ImGui::Checkbox("Overwrite Files", &m_options.output.overwriteFiles);
			ImGui::SameLine();
			ImGui::Checkbox("Via IR", &m_options.output.viaIR);
			if (m_options.output.debugInfoSelection.has_value())
			{
				ImGui::Text("Debug Info Selection");
				ImGui::Checkbox("Location", &m_options.output.debugInfoSelection->location);
				ImGui::SameLine();
				ImGui::Checkbox("Snippet", &m_options.output.debugInfoSelection->snippet);
				ImGui::SameLine();
				ImGui::Checkbox("AST ID", &m_options.output.debugInfoSelection->astID);
			}
			static const char* compiler_states_text[]
				= {"Empty",
				   "Sources Set",
				   "Parsed",
				   "Parsed And Imported",
				   "Analysis Performed",
				   "Compilation Successful"};
			int compiler_states_index = (int) m_options.output.stopAfter;
			if (ImGui::Combo(
					"Stop After", &revert_strings_index, compiler_states_text, IM_ARRAYSIZE(compiler_states_text), 4))
				m_options.output.stopAfter
					= static_cast<solidity::frontend::CompilerStack::State>(compiler_states_index);
			_end();
		}
		//			struct
		//			{
		//				yul::YulStack::Machine targetMachine = yul::YulStack::Machine::EVM;
		//				yul::YulStack::Language inputLanguage = yul::YulStack::Language::StrictAssembly;
		//			} assembly;
		if (_begin("Assembly", true))
		{
			//		solidity::yul::YulStack::Machine
			static const char* input_language_text[] = {"Yul", "Assembly", "Strict Assembly", "EWASM"};
			int input_language_index = (int) m_options.assembly.inputLanguage;
			if (ImGui::Combo(
					"Input Language", &input_language_index, input_language_text, IM_ARRAYSIZE(input_language_text), 4))
				m_options.assembly.inputLanguage = static_cast<solidity::yul::YulStack::Language>(input_language_index);
			static const char* target_machine_text[] = {"EVM", "EWASM"};
			int target_machine_index = (int) m_options.assembly.targetMachine;
			if (ImGui::Combo(
					"Target Machine", &target_machine_index, target_machine_text, IM_ARRAYSIZE(target_machine_text), 4))
				m_options.assembly.targetMachine = static_cast<solidity::yul::YulStack::Machine>(target_machine_index);
			_end();
		}
		//			struct
		//			{
		//				std::map<std::string, util::h160> libraries; // library name -> address
		//			} linker;
		if (_begin("Linker", true))
		{
			for (auto& library: m_options.linker.libraries)
			{
				char value[128];
				memset(&value, 0, sizeof(value));
				memcpy(value, library.second.hex().c_str(), library.second.hex().length());
				ImGui::PushItemWidth(700);
				if (ImGui::InputText(library.first.c_str(), value, sizeof(value)))
					try
					{
						library.second = solidity::util::h160(
							std::string(value),
							solidity::util::h160::ConstructFromStringType::FromHex,
							solidity::util::h160::ConstructFromHashType::AlignRight);
					}
					catch (...)
					{
						library.second = solidity::util::h160();
					}
			}
			_end();
		}
		//			struct
		//			{
		//				util::JsonFormat json;
		//				std::optional<bool> coloredOutput;
		//				bool withErrorIds = false;
		//			} formatting;
		if (_begin("Formatting", true))
		{
			static const char* json_formatting_text[] = {"Compact", "Pretty"};
			int json_formatting_index = (int) m_options.formatting.json.format;
			if (ImGui::
					Combo("JSON", &json_formatting_index, json_formatting_text, IM_ARRAYSIZE(json_formatting_text), 4))
				m_options.formatting.json.format
					= static_cast<solidity::util::JsonFormat::Format>(json_formatting_index);
			ImGui::Checkbox("With Error IDs", &m_options.formatting.withErrorIds);
			if (m_options.formatting.coloredOutput.has_value())
			{
				ImGui::SameLine();
				ImGui::Checkbox("Colored Output", &m_options.formatting.coloredOutput.value());
			}
			_end();
		}
		//			struct
		//			{
		//				CompilerOutputs outputs;
		//				bool estimateGas = false;
		//				std::optional<CombinedJsonRequests> combinedJsonRequests;
		//			} compiler;
		if (_begin("Compiler", true))
		{
			ImGui::Checkbox("Estimate Gas", &m_options.compiler.estimateGas);
			ImGui::Text("Outputs");
			int current = 0;
			for (auto& i: solidity::frontend::CompilerOutputs::componentMap())
			{
				bool checkbox = i.second;
				if (current % 2 == 1)
					ImGui::SameLine();
				ImGui::Checkbox(i.first.c_str(), &checkbox);
				++current;
			}
			ImGui::Text("Combined JSON Requests");
			current = 0;
			for (auto& i: solidity::frontend::CombinedJsonRequests::componentMap())
			{
				bool checkbox = i.second;
				if (current % 2 == 1)
					ImGui::SameLine();
				ImGui::Checkbox(i.first.c_str(), &checkbox);
				++current;
			}
			_end();
		}
		//			struct
		//			{
		//				CompilerStack::MetadataHash hash = CompilerStack::MetadataHash::IPFS;
		//				bool literalSources = false;
		//			} metadata;
		if (_begin("Metadata", true))
		{
			static const char* metadata_hash_type[] = {"IPFS", "BZZR1", "None"};
			int metadata_hash_type_selection = (int) m_options.metadata.hash;
			if (ImGui::Combo(
					"Metadata Hash Type",
					&metadata_hash_type_selection,
					metadata_hash_type,
					IM_ARRAYSIZE(metadata_hash_type),
					4))
				m_options.metadata.hash
					= static_cast<solidity::frontend::CompilerStack::MetadataHash>(metadata_hash_type_selection);
			ImGui::Checkbox("Literal Sources", &m_options.metadata.literalSources);
			_end();
		}
		//			struct
		//			{
		//				bool enabled = false;
		//				std::optional<unsigned> expectedExecutionsPerDeployment;
		//				bool noOptimizeYul = false;
		//				std::optional<std::string> yulSteps;
		//			} optimizer;
		if (_begin("Optimizer", true))
		{
			ImGui::Checkbox("Enabled", &m_options.optimizer.enabled);
			ImGui::SameLine();
			ImGui::Checkbox("No Optimize YUL", &m_options.optimizer.noOptimizeYul);
			if (m_options.optimizer.expectedExecutionsPerDeployment.has_value())
			{
				ImGui::InputInt(
					"Expected Executions Per Deployment",
					(int*) &m_options.optimizer.expectedExecutionsPerDeployment.value());
				if (static_cast<int>(m_options.optimizer.expectedExecutionsPerDeployment.value()) < 0)
					m_options.optimizer.expectedExecutionsPerDeployment = 0;
			}
			if (m_options.optimizer.yulSteps.has_value())
			{
				char input[1024];
				memset(input, 0x00, sizeof(input));
				memcpy(
					input, m_options.optimizer.yulSteps.value().c_str(), m_options.optimizer.yulSteps.value().length());
				if (ImGui::InputText("YUL Steps", input, sizeof(input)))
					m_options.optimizer.yulSteps = std::string(input);
			}
			_end();
		}
		//			struct
		//			{
		//				bool initialize = false;
		//				ModelCheckerSettings settings;
		//			} modelChecker;
		if (_begin("Model Checker", true))
		{
			ImGui::Checkbox("Initialize", &m_options.modelChecker.initialize);
			ImGui::Checkbox("Div Mod No Slacks", &m_options.modelChecker.settings.divModNoSlacks);
			ImGui::SameLine();
			ImGui::Checkbox("Show Unproved", &m_options.modelChecker.settings.showUnproved);
			static const char* engine_text[] = {"All", "BMC", "CHC", "None"};
			int engine_index = -1;
			if (m_options.modelChecker.settings.engine.bmc == true
				&& m_options.modelChecker.settings.engine.chc == true)
				engine_index = 0;
			else if (
				m_options.modelChecker.settings.engine.bmc == true
				&& m_options.modelChecker.settings.engine.chc == false)
				engine_index = 1;
			else if (
				m_options.modelChecker.settings.engine.bmc == false
				&& m_options.modelChecker.settings.engine.chc == true)
				engine_index = 2;
			else if (
				m_options.modelChecker.settings.engine.bmc == false
				&& m_options.modelChecker.settings.engine.chc == false)
				engine_index = 3;
			if (ImGui::Combo("Engine", &engine_index, engine_text, IM_ARRAYSIZE(engine_text), 4))
				m_options.modelChecker.settings.engine
					= solidity::frontend::ModelCheckerEngine::fromString(
						  boost::to_lower_copy(std::string(engine_text[engine_index])))
						  .value();
			//			Contract, Reentrancy, all, default
			static const char* invariants_text[] = {"All", "Contract", "Reentrancy", "Default"};
			int invariants_selection = -1;
			if (m_options.modelChecker.settings.invariants == solidity::frontend::ModelCheckerInvariants::All())
				invariants_selection = 0;
			else if (
				m_options.modelChecker.settings.invariants
				== solidity::frontend::ModelCheckerInvariants::fromString("contract"))
				invariants_selection = 1;
			else if (
				m_options.modelChecker.settings.invariants
				== solidity::frontend::ModelCheckerInvariants::fromString("reentrancy"))
				invariants_selection = 2;
			else if (
				m_options.modelChecker.settings.invariants == solidity::frontend::ModelCheckerInvariants::Default())
				invariants_selection = 3;
			if (ImGui::Combo("Invariants", &invariants_selection, invariants_text, IM_ARRAYSIZE(invariants_text), 4))
				m_options.modelChecker.settings.invariants
					= solidity::frontend::ModelCheckerInvariants::fromString(
						  boost::to_lower_copy(std::string(invariants_text[invariants_selection])))
						  .value();
			static const char* solver_text[]
				= {"All",
				   "CVC4",
				   "SMTLIB2",
				   "Z3"}; // and "None", todo: ask leo about smtutil::SMTSolverChoice::fromString
			int solver_index = -1;
			if (m_options.modelChecker.settings.solvers.cvc4 == true
				&& m_options.modelChecker.settings.solvers.smtlib2 == true
				&& m_options.modelChecker.settings.solvers.z3 == true)
				solver_index = 0;
			else if (m_options.modelChecker.settings.solvers.cvc4 == true)
				solver_index = 1;
			else if (m_options.modelChecker.settings.solvers.smtlib2 == true)
				solver_index = 2;
			else if (m_options.modelChecker.settings.solvers.z3 == true)
				solver_index = 3;
			// todo: ask leo
			//			else if (m_options.modelChecker.settings.solvers.cvc4 == false &&
			// m_options.modelChecker.settings.solvers.smtlib2 == false && m_options.modelChecker.settings.solvers.z3
			//== false) 				solver_index = 4;
			if (ImGui::Combo("Solver", &solver_index, solver_text, IM_ARRAYSIZE(solver_text), 4))
				m_options.modelChecker.settings.solvers
					= solidity::smtutil::SMTSolverChoice::fromString(
						  boost::to_lower_copy(std::string(solver_text[solver_index])))
						  .value();

			_end();
		}
	}

private:
	solidity::frontend::CommandLineOptions& m_options;
};

} // namespace solidity::ui
