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

#include <libevmasm/Ethdebug.h>

#include <libevmasm/EthdebugSchema.h>

#include <libsolutil/Keccak256.h>

#include <fmt/format.h>

#include <range/v3/algorithm/any_of.hpp>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::evmasm::ethdebug;

namespace
{

schema::program::Instruction::Operation instructionOperation(Assembly const& _assembly, LinkerObject const& _linkerObject, size_t const _start, size_t const _end)
{
	solAssert(_end <= _linkerObject.bytecode.size());
	solAssert(_start < _end);
	schema::program::Instruction::Operation operation;
	operation.mnemonic = instructionInfo(static_cast<Instruction>(_linkerObject.bytecode[_start]), _assembly.evmVersion()).name;
	static size_t constexpr instructionSize = 1;
	if (_start + instructionSize < _end)
	{
		bytes const argumentData(
			_linkerObject.bytecode.begin() + static_cast<std::ptrdiff_t>(_start) + instructionSize,
			_linkerObject.bytecode.begin() + static_cast<std::ptrdiff_t>(_end)
		);
		solAssert(!argumentData.empty());
		operation.arguments = {{schema::data::HexValue{argumentData}}};
	}
	return operation;
}

schema::materials::SourceRange::Range locationRange(langutil::SourceLocation const& _location)
{
	return {
		.length = schema::data::Unsigned{_location.end - _location.start},
		.offset = schema::data::Unsigned{_location.start}
	};
}

schema::materials::Reference sourceReference(unsigned _sourceID)
{
	return {
		.id = schema::materials::ID{_sourceID},
		.type = std::nullopt
	};
}

std::optional<schema::program::Context> instructionContext(AssemblyItems const& _codeSection, size_t _assemblyItemIndex, unsigned _sourceID)
{
	solAssert(_assemblyItemIndex < _codeSection.size());
	langutil::SourceLocation const& location = _codeSection.at(_assemblyItemIndex).location();
	if (!location.isValid())
		return std::nullopt;

	return schema::program::Context{
		schema::materials::SourceRange{
			.source = sourceReference(_sourceID),
			.range = locationRange(location)
		},
		std::nullopt,
		std::nullopt
	};
}

std::vector<schema::program::Instruction> programInstructions(Assembly const& _assembly, LinkerObject const& _linkerObject, unsigned const _sourceID)
{
	auto const& locations = _linkerObject.codeSectionLocation;
	AssemblyItems const& items = _assembly.items();

	std::vector<schema::program::Instruction> instructions;
	instructions.reserve(items.size());

	bool const codeSectionContainsVerbatim = ranges::any_of(
		items,
		[](auto const& _instruction) { return _instruction.type() == VerbatimBytecode; }
	);
	solUnimplementedAssert(!codeSectionContainsVerbatim, "Verbatim bytecode is currently not supported by ethdebug.");

	for (auto const& currentInstruction: locations.instructionLocations)
	{
		size_t const start = currentInstruction.start;
		size_t const end = currentInstruction.end;

		// some instructions do not contribute to the bytecode
		if (start == end)
			continue;

		instructions.emplace_back(schema::program::Instruction{
			.offset = schema::data::Unsigned{start},
			.operation = instructionOperation(_assembly, _linkerObject, start, end),
			.context = instructionContext(items, currentInstruction.assemblyItemIndex, _sourceID)
		});
	}

	return instructions;
}

std::string lengthPrefixed(std::string_view _value)
{
	return fmt::format("{}:{}", _value.size(), _value);
}

std::string compilationID(std::vector<Source> const& _sources)
{
	// Keep this independent from VersionString build metadata.
	// TODO: take compilation flags into account. Once compiler settings are threaded
	// into ETHDebug resources, include a normalized settings object here.
	std::string rawIDInput = "ethdebug-solc-compilation-v1";
	rawIDInput += lengthPrefixed(std::to_string(_sources.size()));

	for (auto const& source: _sources)
	{
		rawIDInput += lengthPrefixed(std::to_string(source.id));
		rawIDInput += lengthPrefixed(source.path);
		rawIDInput += lengthPrefixed(source.contents);
		rawIDInput += lengthPrefixed(source.language);
	}

	return fmt::format("solc-{}", util::keccak256(rawIDInput).hex());
}

schema::materials::Source materialSource(Source const& _source)
{
	schema::materials::Source result;
	result.id = schema::materials::ID{_source.id};
	result.path = _source.path;
	result.contents = _source.contents;
	result.encoding = std::nullopt;
	result.language = _source.language;
	return result;
}

schema::materials::Compilation materialCompilation(std::vector<Source> const& _sources, std::string_view _version)
{
	std::vector<schema::materials::Source> sources;
	sources.reserve(_sources.size());
	for (auto const& source: _sources)
		sources.emplace_back(materialSource(source));

	schema::materials::Compilation result;
	result.id = schema::materials::ID{compilationID(_sources)};
	result.compiler.name = "solc";
	result.compiler.version = std::string{_version};
	result.settings = std::nullopt;
	result.sources = std::move(sources);
	return result;
}

} // anonymous namespace

Json ethdebug::program(std::string_view _name, unsigned _sourceID, Assembly const& _assembly, LinkerObject const& _linkerObject)
{
	return schema::Program{
		.compilation = std::nullopt,
		.contract = {
			.name = std::string{_name},
			.definition = {
				.source = {
					.id = {_sourceID},
					.type = std::nullopt
				},
				.range = std::nullopt
			}
		},
		.environment = _assembly.isCreation() ? schema::Program::Environment::CREATE : schema::Program::Environment::CALL,
		.context = std::nullopt,
		.instructions = programInstructions(_assembly, _linkerObject, _sourceID)
	};
}

Json ethdebug::resources(
	std::vector<Source> const& _sources,
	std::string_view _version,
	std::map<std::string, schema::type::Type> _types,
	std::map<std::string, schema::pointer::Template> _pointers
)
{
	schema::info::Resources result;
	result.compilation = materialCompilation(_sources, _version);
	result.types = std::move(_types);
	result.pointers = std::move(_pointers);
	return result;
}

Json ethdebug::compilation(std::vector<Source> const& _sources, std::string_view _version)
{
	return materialCompilation(_sources, _version);
}
