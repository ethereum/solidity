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
/**
 * Unit tests for the compiler itself.
 */

#include <test/libsolidity/AnalysisFramework.h>
#include <test/Metadata.h>
#include <test/Common.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Numeric.h>

#include <boost/test/unit_test.hpp>

#include <regex>

using namespace solidity::test;

namespace solidity::frontend::test
{

namespace
{

/// Returns true if @a _reason appears anywhere in @a _assembly: as contiguous ASCII bytes, as a
/// standalone hex constant that decodes to it directly (e.g. legacy codegen's
/// `0x7265766572745f61...` for "revert_a", zero-padded), or as a bit-packed `shl(shift, value)`
/// constant - a Yul optimizer representation for a mostly-zero 256-bit word that a plain
/// substring search would miss entirely.
bool assemblyContainsString(std::string const& _assembly, std::string const& _reason)
{
	if (_assembly.find(_reason) != std::string::npos)
		return true;

	static std::regex const hexPattern(R"(0x([0-9a-fA-F]+))");
	for (
		auto it = std::sregex_iterator(_assembly.begin(), _assembly.end(), hexPattern);
		it != std::sregex_iterator();
		++it
	)
	{
		std::string hex = (*it)[1].str();
		if (hex.size() % 2 != 0)
			hex = "0" + hex;
		bytes const decodedBytes = util::fromHex(hex);
		std::string const decoded(decodedBytes.begin(), decodedBytes.end());
		if (decoded.find(_reason) != std::string::npos)
			return true;
	}

	static std::regex const shlPattern(R"(shl\(0x([0-9a-fA-F]+),\s*0x([0-9a-fA-F]+)\))");
	for (
		auto it = std::sregex_iterator(_assembly.begin(), _assembly.end(), shlPattern);
		it != std::sregex_iterator();
		++it
	)
	{
		unsigned shift = static_cast<unsigned>(std::stoul((*it)[1].str(), nullptr, 16));
		u256 value(std::string("0x") + (*it)[2].str());
		bytes packed = toBigEndian(value << shift);
		std::string const packedString(packed.begin(), packed.end());
		if (packedString.find(_reason) != std::string::npos)
			return true;
	}
	return false;
}

}

class SolidityCompilerFixture: protected AnalysisFramework
{
	void setupCompiler(CompilerStack& _compiler) override
	{
		AnalysisFramework::setupCompiler(_compiler);

		// FIXME: This test was probably supposed to respect CommonOptions::get().optimize but
		// due to a bug it was always running with optimizer disabled and it does not pass with it.
		_compiler.setOptimiserSettings(false);
	}
};

BOOST_FIXTURE_TEST_SUITE(SolidityCompiler, SolidityCompilerFixture)

BOOST_AUTO_TEST_CASE(does_not_include_creation_time_only_internal_functions)
{
	char const* sourceCode = R"(
		contract C {
			uint x;
			constructor() { f(); }
			function f() internal { unchecked { for (uint i = 0; i < 10; ++i) x += 3 + i; } }
		}
	)";

	runFramework(sourceCode, PipelineStage::Compilation);
	BOOST_REQUIRE_MESSAGE(
		pipelineSuccessful(),
		"Contract compilation failed:\n" + formatErrors(filteredErrors(), true /* _colored */)
	);

	bytes const& creationBytecode = solidity::test::bytecodeSansMetadata(compiler().object("C").bytecode);
	bytes const& runtimeBytecode = solidity::test::bytecodeSansMetadata(compiler().runtimeObject("C").bytecode);
	BOOST_CHECK(creationBytecode.size() >= 90);
	BOOST_CHECK(creationBytecode.size() <= 120);
	auto evmVersion = solidity::test::CommonOptions::get().evmVersion();
	unsigned threshold = evmVersion.hasPush0() ? 9 : 10;
	BOOST_CHECK(runtimeBytecode.size() >= threshold);
	BOOST_CHECK(runtimeBytecode.size() <= 30);
}

BOOST_AUTO_TEST_SUITE_END()

class RevertStringsStripFixture: protected AnalysisFramework
{
	void setupCompiler(CompilerStack& _compiler) override
	{
		AnalysisFramework::setupCompiler(_compiler);
		_compiler.setRevertStringBehaviour(RevertStrings::Strip);
	}
};

BOOST_FIXTURE_TEST_SUITE(RevertStringsStrip, RevertStringsStripFixture)

BOOST_AUTO_TEST_CASE(reason_expression_evaluation_does_not_leak_string_data)
{
	// Reason expressions below still have to be evaluated (they may panic), but since the encoded
	// string value is discarded by --revert-strings strip, none of the literal text should ever end
	// up materialized in memory, and therefore should never appear in the bytecode either.
	char const* sourceCode = R"(
		contract C {
			string constant CONSTANT_REASON = (new bytes(0))[0] == bytes1(0) ? "constant_a" : "constant_b";

			function requireReasonConditionalIsEvaluated() external pure {
				require(true, (new bytes(0))[0] == bytes1(0) ? "require_a" : "require_b");
			}

			function revertReasonConditionalIsEvaluated() external pure {
				revert(type(uint8).max + 1 == 0 ? "revert_a" : "revert_b");
			}

			function revertReasonConstantIsEvaluated() external pure {
				revert(CONSTANT_REASON);
			}

			function revertReasonNestedConditionalIsEvaluated() external pure {
				revert(true ? (1 / type(uint8).min == 0 ? "nested_a" : "nested_b") : "nested_c");
			}
		}
	)";

	BOOST_REQUIRE(runFramework(sourceCode, PipelineStage::Compilation));
	BOOST_REQUIRE_MESSAGE(
		pipelineSuccessful(),
		"Contract compilation failed:\n" + formatErrors(filteredErrors(), true /* _colored */)
	);

	std::string const assembly = compiler().assemblyString("C", {});

	for (std::string const reason: {"require_a", "require_b", "revert_a", "revert_b", "constant_a", "constant_b", "nested_a", "nested_b", "nested_c"})
		BOOST_CHECK_MESSAGE(
			!assemblyContainsString(assembly, reason),
			"Revert reason string \"" + reason + "\" leaked into the bytecode despite --revert-strings strip."
		);
}

BOOST_AUTO_TEST_SUITE_END()

class ViaIRRevertStringsStripFixture: protected AnalysisFramework
{
	void setupCompiler(CompilerStack& _compiler) override
	{
		AnalysisFramework::setupCompiler(_compiler);
		_compiler.setViaIR(true);
		_compiler.setOptimiserSettings(true);
		_compiler.setRevertStringBehaviour(RevertStrings::Strip);
	}
};

BOOST_FIXTURE_TEST_SUITE(ViaIRRevertStringsStrip, ViaIRRevertStringsStripFixture)

BOOST_AUTO_TEST_CASE(reason_expression_evaluation_does_not_leak_string_data)
{
	// Same intent as RevertStringsStrip/reason_expression_evaluation_does_not_leak_string_data, but
	// compiled via-ir with the optimizer on. Deliberately avoids an array bounds check (e.g.
	// `(new bytes(0))[0]`) as a require() reason's condition: unlike revert(reason), or require()
	// reasons whose condition is a simple arithmetic check (as used here), a require() reason whose
	// condition involves an array bounds check is NOT fully eliminated by the via-ir + --optimize
	// pipeline (tracked separately, independent of this fix - see branch
	// via-ir-require-reason-not-stripped).
	char const* sourceCode = R"(
		contract C {
			enum MyEnum { A, B }
			string constant CONSTANT_REASON = MyEnum(type(uint8).max) == MyEnum.A ? "constant_a" : "constant_b";

			function requireReasonConditionalIsEvaluated() external pure {
				require(true, type(uint8).max + 1 == 0 ? "require_a" : "require_b");
			}

			function revertReasonConditionalIsEvaluated() external pure {
				revert(1 / type(uint8).min == 0 ? "revert_a" : "revert_b");
			}

			function revertReasonConstantIsEvaluated() external pure {
				revert(CONSTANT_REASON);
			}

			function revertReasonNestedConditionalIsEvaluated() external pure {
				revert(true ? (type(uint8).max + 1 == 0 ? "nested_a" : "nested_b") : "nested_c");
			}
		}
	)";

	BOOST_REQUIRE(runFramework(sourceCode, PipelineStage::Compilation));
	BOOST_REQUIRE_MESSAGE(
		pipelineSuccessful(),
		"Contract compilation failed:\n" + formatErrors(filteredErrors(), true /* _colored */)
	);

	std::string const assembly = compiler().assemblyString("C", {});

	for (std::string const reason: {"require_a", "require_b", "revert_a", "revert_b", "constant_a", "constant_b", "nested_a", "nested_b", "nested_c"})
		BOOST_CHECK_MESSAGE(
			!assemblyContainsString(assembly, reason),
			"Revert reason string \"" + reason + "\" leaked into the via-ir bytecode despite --revert-strings strip."
		);
}

BOOST_AUTO_TEST_SUITE_END()

}
