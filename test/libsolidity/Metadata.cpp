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
 * @date 2017
 * Unit tests for the metadata output.
 */

#include <test/Metadata.h>
#include <test/Common.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/Version.h>
#include <libsolutil/SwarmHash.h>
#include <libsolutil/IpfsHash.h>
#include <libsolutil/JSON.h>

#include <boost/test/unit_test.hpp>


namespace solidity::frontend::test
{

namespace
{

std::map<std::string, std::string> requireParsedCBORMetadata(bytes const& _bytecode, CompilerStack::MetadataFormat _metadataFormat)
{
	bytes cborMetadata = solidity::test::onlyMetadata(_bytecode);
	if (_metadataFormat != CompilerStack::MetadataFormat::NoMetadata)
	{
		BOOST_REQUIRE(!cborMetadata.empty());
		std::optional<std::map<std::string, std::string>> tmp = solidity::test::parseCBORMetadata(cborMetadata);
		BOOST_REQUIRE(tmp);
		return *tmp;
	}
	BOOST_REQUIRE(cborMetadata.empty());
	return {};
}

std::optional<std::string> compileAndCheckLicenseMetadata(std::string const& _contractName, char const* _sourceCode)
{
	CompilerStack compilerStack;
	compilerStack.setSources({{"A.sol", _sourceCode}});
	BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");

	std::string const& serialisedMetadata = compilerStack.metadata(_contractName);
	Json metadata;
	BOOST_REQUIRE(util::jsonParseStrict(serialisedMetadata, metadata));
	BOOST_CHECK(solidity::test::isValidMetadata(metadata));

	BOOST_CHECK_EQUAL(metadata["sources"].size(), 1);
	BOOST_REQUIRE(metadata["sources"].contains("A.sol"));

	if (metadata["sources"]["A.sol"].contains("license"))
	{
		BOOST_REQUIRE(metadata["sources"]["A.sol"]["license"].is_string());
		return metadata["sources"]["A.sol"]["license"].get<std::string>();
	}
	else
		return std::nullopt;
}

}

BOOST_AUTO_TEST_SUITE(Metadata)

BOOST_AUTO_TEST_CASE(metadata_stamp)
{
	// Check that the metadata stamp is at the end of the runtime bytecode.
	char const* sourceCode = R"(
		pragma solidity >=0.0;
		pragma experimental __testOnlyAnalysis;
		contract test {
			function g(function(uint) external returns (uint) x) public {}
		}
	)";
	for (auto metadataFormat: std::set<CompilerStack::MetadataFormat>{
		CompilerStack::MetadataFormat::NoMetadata,
		CompilerStack::MetadataFormat::WithReleaseVersionTag,
		CompilerStack::MetadataFormat::WithPrereleaseVersionTag
	})
		for (auto metadataHash: std::set<CompilerStack::MetadataHash>{
			CompilerStack::MetadataHash::IPFS,
			CompilerStack::MetadataHash::Bzzr1,
			CompilerStack::MetadataHash::None
		})
		{
			CompilerStack compilerStack;
			compilerStack.setMetadataFormat(metadataFormat);
			compilerStack.setSources({{"", sourceCode}});
			compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
			compilerStack.setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
			compilerStack.setMetadataHash(metadataHash);
			BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");
			bytes const& bytecode = compilerStack.runtimeObject("test").bytecode;
			std::string const& metadata = compilerStack.metadata("test");
			BOOST_CHECK(solidity::test::isValidMetadata(metadata));

			auto const cborMetadata = requireParsedCBORMetadata(bytecode, metadataFormat);
			if (metadataHash == CompilerStack::MetadataHash::None)
				BOOST_CHECK(cborMetadata.size() == (metadataFormat == CompilerStack::MetadataFormat::NoMetadata ? 0 : 1));
			else
			{
				bytes hash;
				std::string hashMethod;
				if (metadataHash == CompilerStack::MetadataHash::IPFS)
				{
					hash = util::ipfsHash(metadata);
					BOOST_REQUIRE(hash.size() == 34);
					hashMethod = "ipfs";
				}
				else
				{
					hash = util::bzzr1Hash(metadata).asBytes();
					BOOST_REQUIRE(hash.size() == 32);
					hashMethod = "bzzr1";
				}

				if (metadataFormat != CompilerStack::MetadataFormat::NoMetadata)
				{
					BOOST_CHECK(cborMetadata.size() == 2);
					BOOST_CHECK(cborMetadata.count(hashMethod) == 1);
					BOOST_CHECK(cborMetadata.at(hashMethod) == util::toHex(hash));
				}
			}

			if (metadataFormat == CompilerStack::MetadataFormat::NoMetadata)
				BOOST_CHECK(cborMetadata.count("solc") == 0);
			else
			{
				BOOST_CHECK(cborMetadata.count("solc") == 1);
				if (metadataFormat == CompilerStack::MetadataFormat::WithReleaseVersionTag)
					BOOST_CHECK(cborMetadata.at("solc") == util::toHex(VersionCompactBytes));
				else
					BOOST_CHECK(cborMetadata.at("solc") == VersionStringStrict);
			}
		}
}

BOOST_AUTO_TEST_CASE(metadata_stamp_experimental)
{
	// Check that the metadata stamp is at the end of the runtime bytecode.
	char const* sourceCode = R"(
		pragma solidity >=0.0;
		pragma experimental __test;
		contract test {
			function g(function(uint) external returns (uint) x) public {}
		}
	)";
	for (auto metadataFormat: std::set<CompilerStack::MetadataFormat>{
			CompilerStack::MetadataFormat::NoMetadata,
			CompilerStack::MetadataFormat::WithReleaseVersionTag,
			CompilerStack::MetadataFormat::WithPrereleaseVersionTag
	})
		for (auto metadataHash: std::set<CompilerStack::MetadataHash>{
			CompilerStack::MetadataHash::IPFS,
			CompilerStack::MetadataHash::Bzzr1,
			CompilerStack::MetadataHash::None
		})
		{
			CompilerStack compilerStack;
			compilerStack.setMetadataFormat(metadataFormat);
			compilerStack.setSources({{"", sourceCode}});
			compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
			compilerStack.setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
			compilerStack.setMetadataHash(metadataHash);
			BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");
			bytes const& bytecode = compilerStack.runtimeObject("test").bytecode;
			std::string const& metadata = compilerStack.metadata("test");
			BOOST_CHECK(solidity::test::isValidMetadata(metadata));

			auto const cborMetadata = requireParsedCBORMetadata(bytecode, metadataFormat);
			if (metadataHash == CompilerStack::MetadataHash::None)
				BOOST_CHECK(cborMetadata.size() == (metadataFormat == CompilerStack::MetadataFormat::NoMetadata ? 0 : 2));
			else
			{
				bytes hash;
				std::string hashMethod;
				if (metadataHash == CompilerStack::MetadataHash::IPFS)
				{
					hash = util::ipfsHash(metadata);
					BOOST_REQUIRE(hash.size() == 34);
					hashMethod = "ipfs";
				}
				else
				{
					hash = util::bzzr1Hash(metadata).asBytes();
					BOOST_REQUIRE(hash.size() == 32);
					hashMethod = "bzzr1";
				}

				if (metadataFormat != CompilerStack::MetadataFormat::NoMetadata)
				{
					BOOST_CHECK(cborMetadata.size() == 3);
					BOOST_CHECK(cborMetadata.count(hashMethod) == 1);
					BOOST_CHECK(cborMetadata.at(hashMethod) == util::toHex(hash));
				}
			}

			if (metadataFormat == CompilerStack::MetadataFormat::NoMetadata)
				BOOST_CHECK(cborMetadata.count("solc") == 0);
			else
			{
				BOOST_CHECK(cborMetadata.count("solc") == 1);
				if (metadataFormat == CompilerStack::MetadataFormat::WithReleaseVersionTag)
					BOOST_CHECK(cborMetadata.at("solc") == util::toHex(VersionCompactBytes));
				else
					BOOST_CHECK(cborMetadata.at("solc") == VersionStringStrict);
				BOOST_CHECK(cborMetadata.count("experimental") == 1);
				BOOST_CHECK(cborMetadata.at("experimental") == "true");
			}
		}
}

BOOST_AUTO_TEST_CASE(metadata_relevant_sources)
{
	CompilerStack compilerStack;
	char const* sourceCodeA = R"(
		pragma solidity >=0.0;
		contract A {
			function g(function(uint) external returns (uint) x) public {}
		}
	)";
	char const* sourceCodeB = R"(
		pragma solidity >=0.0;
		contract B {
			function g(function(uint) external returns (uint) x) public {}
		}
	)";
	compilerStack.setSources({
		{"A", sourceCodeA},
		{"B", sourceCodeB},
	});
	compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
	compilerStack.setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
	BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");

	std::string const& serialisedMetadata = compilerStack.metadata("A");
	Json metadata;
	BOOST_REQUIRE(util::jsonParseStrict(serialisedMetadata, metadata));
	BOOST_CHECK(solidity::test::isValidMetadata(metadata));

	BOOST_CHECK_EQUAL(metadata["sources"].size(), 1);
	BOOST_CHECK(metadata["sources"].contains("A"));
}

BOOST_AUTO_TEST_CASE(metadata_relevant_sources_imports)
{
	CompilerStack compilerStack;
	char const* sourceCodeA = R"(
		pragma solidity >=0.0;
		contract A {
			function g(function(uint) external returns (uint) x) public virtual {}
		}
	)";
	char const* sourceCodeB = R"(
		pragma solidity >=0.0;
		import "./A";
		contract B is A {
			function g(function(uint) external returns (uint) x) public virtual override {}
		}
	)";
	char const* sourceCodeC = R"(
		pragma solidity >=0.0;
		import "./B";
		contract C is B {
			function g(function(uint) external returns (uint) x) public override {}
		}
	)";
	compilerStack.setSources({
		{"A", sourceCodeA},
		{"B", sourceCodeB},
		{"C", sourceCodeC}
	});
	compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
	compilerStack.setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
	BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");

	std::string const& serialisedMetadata = compilerStack.metadata("C");
	Json metadata;
	BOOST_REQUIRE(util::jsonParseStrict(serialisedMetadata, metadata));
	BOOST_CHECK(solidity::test::isValidMetadata(metadata));

	BOOST_CHECK_EQUAL(metadata["sources"].size(), 3);
	BOOST_CHECK(metadata["sources"].contains("A"));
	BOOST_CHECK(metadata["sources"].contains("B"));
	BOOST_CHECK(metadata["sources"].contains("C"));
}

BOOST_AUTO_TEST_CASE(metadata_useLiteralContent)
{
	// Check that the metadata contains "useLiteralContent"
	char const* sourceCode = R"(
		pragma solidity >=0.0;
		contract test {
		}
	)";

	auto check = [](char const* _src, bool _literal)
	{
		CompilerStack compilerStack;
		compilerStack.setSources({{"", _src}});
		compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
		compilerStack.setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
		compilerStack.useMetadataLiteralSources(_literal);
		BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");
		std::string metadata_str = compilerStack.metadata("test");
		Json metadata;
		BOOST_REQUIRE(util::jsonParseStrict(metadata_str, metadata));
		BOOST_CHECK(solidity::test::isValidMetadata(metadata));
		BOOST_CHECK(metadata.contains("settings"));
		BOOST_CHECK(metadata["settings"].contains("metadata"));
		BOOST_CHECK(metadata["settings"]["metadata"].contains("bytecodeHash"));
		if (_literal)
		{
			BOOST_CHECK(metadata["settings"]["metadata"].contains("useLiteralContent"));
			BOOST_CHECK(metadata["settings"]["metadata"]["useLiteralContent"].get<bool>());
		}
	};

	check(sourceCode, true);
	check(sourceCode, false);
}

BOOST_AUTO_TEST_CASE(metadata_viair)
{
	char const* sourceCode = R"(
		pragma solidity >=0.0;
		contract test {
		}
	)";

	auto check = [](char const* _src, bool _viaIR)
	{
		CompilerStack compilerStack;
		compilerStack.setSources({{"", _src}});
		compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
		compilerStack.setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
		compilerStack.setViaIR(_viaIR);
		BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");

		Json metadata;
		BOOST_REQUIRE(util::jsonParseStrict(compilerStack.metadata("test"), metadata));
		BOOST_CHECK(solidity::test::isValidMetadata(metadata));
		BOOST_CHECK(metadata.contains("settings"));
		if (_viaIR)
		{
			BOOST_CHECK(metadata["settings"].contains("viaIR"));
			BOOST_CHECK(metadata["settings"]["viaIR"].get<bool>());
		}
		else
			BOOST_CHECK(!metadata["settings"].contains("viaIR"));

		BOOST_CHECK(compilerStack.cborMetadata("test") == compilerStack.cborMetadata("test", _viaIR));
		BOOST_CHECK(compilerStack.cborMetadata("test") != compilerStack.cborMetadata("test", !_viaIR));

		std::map<std::string, std::string> const parsedCBORMetadata = requireParsedCBORMetadata(
			compilerStack.runtimeObject("test").bytecode,
			CompilerStack::MetadataFormat::WithReleaseVersionTag
		);

		BOOST_CHECK(parsedCBORMetadata.count("experimental") == 0);
	};

	check(sourceCode, true);
	check(sourceCode, false);
}

BOOST_AUTO_TEST_CASE(metadata_revert_strings)
{
	CompilerStack compilerStack;
	char const* sourceCodeA = R"(
		pragma solidity >=0.0;
		contract A {
		}
	)";
	compilerStack.setSources({{"A", sourceCodeA}});
	compilerStack.setRevertStringBehaviour(RevertStrings::Strip);
	BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");

	std::string const& serialisedMetadata = compilerStack.metadata("A");
	Json metadata;
	BOOST_REQUIRE(util::jsonParseStrict(serialisedMetadata, metadata));
	BOOST_CHECK(solidity::test::isValidMetadata(metadata));

	BOOST_CHECK_EQUAL(metadata["settings"]["debug"]["revertStrings"], "strip");
}

BOOST_AUTO_TEST_CASE(metadata_optimiser_sequence)
{
	char const* sourceCode = R"(
		pragma solidity >=0.0;
		contract C {
		}
	)";

	std::vector<std::tuple<std::string, std::string>> sequences =
	{
		// {"<optimizer sequence>", "<optimizer cleanup sequence>"}
		{"", ""},
		{"", "fDn"},
		{"dhfoDgvulfnTUtnIf", "" },
		{"dhfoDgvulfnTUtnIf", "fDn"}
	};

	auto check = [sourceCode](std::string const& _optimizerSequence, std::string const& _optimizerCleanupSequence)
	{
		OptimiserSettings optimizerSettings = OptimiserSettings::minimal();
		optimizerSettings.runYulOptimiser = true;
		optimizerSettings.yulOptimiserSteps = _optimizerSequence;
		optimizerSettings.yulOptimiserCleanupSteps = _optimizerCleanupSequence;
		CompilerStack compilerStack;
		compilerStack.setSources({{"", sourceCode}});
		compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
		compilerStack.setOptimiserSettings(optimizerSettings);

		BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");

		std::string const& serialisedMetadata = compilerStack.metadata("C");
		Json metadata;
		BOOST_REQUIRE(util::jsonParseStrict(serialisedMetadata, metadata));
		BOOST_CHECK(solidity::test::isValidMetadata(metadata));
		BOOST_CHECK(metadata["settings"]["optimizer"].contains("details"));
		BOOST_CHECK(metadata["settings"]["optimizer"]["details"].contains("yulDetails"));
		BOOST_CHECK(metadata["settings"]["optimizer"]["details"]["yulDetails"].contains("optimizerSteps"));

		std::string const metadataOptimizerSteps = metadata["settings"]["optimizer"]["details"]["yulDetails"]["optimizerSteps"].get<std::string>();
		std::string const expectedMetadataOptimiserSteps = _optimizerSequence + ":" + _optimizerCleanupSequence;
		BOOST_CHECK_EQUAL(metadataOptimizerSteps, expectedMetadataOptimiserSteps);
	};

	for (auto const& [sequence, cleanupSequence] : sequences)
		check(sequence, cleanupSequence);
}

BOOST_AUTO_TEST_CASE(metadata_license_missing)
{
	char const* sourceCode = R"(
		pragma solidity >=0.0;
		contract C {
		}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == std::nullopt);
}

BOOST_AUTO_TEST_CASE(metadata_license_gpl3)
{
	// Can't use a raw string here due to the stylechecker.
	char const* sourceCode =
		"// NOTE: we also add trailing whitespace after the license, to see it is trimmed.\n"
		"// SPDX-License-Identifier: GPL-3.0    \n"
		"pragma solidity >=0.0;\n"
		"contract C {\n"
		"}\n";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_whitespace_before_spdx)
{
	char const* sourceCode = R"(
		//     SPDX-License-Identifier: GPL-3.0
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_whitespace_after_colon)
{
	char const* sourceCode = R"(
		// SPDX-License-Identifier:    GPL-3.0
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_gpl3_or_apache2)
{
	char const* sourceCode = R"(
		// SPDX-License-Identifier: GPL-3.0 OR Apache-2.0
		pragma solidity >=0.0;
		contract C {
		}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0 OR Apache-2.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_bidi_marks)
{
	char const* sourceCode =
		"// \xE2\x80\xAE""0.3-LPG :reifitnedI-esneciL-XDPS\xE2\x80\xAC\n"
		"// NOTE: The text above is reversed using Unicode directional marks. In raw form it would look like this:\n"
		"// <LRO>0.3-LPG :reifitnedI-esneciL-XDPS<PDF>\n"
		"contract C {}\n";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == std::nullopt);
}

BOOST_AUTO_TEST_CASE(metadata_license_bottom)
{
	char const* sourceCode = R"(
		contract C {}
		// SPDX-License-Identifier: GPL-3.0
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_cr_endings)
{
	char const* sourceCode =
		"// SPDX-License-Identifier: GPL-3.0\r"
		"contract C {}\r";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_crlf_endings)
{
	char const* sourceCode =
		"// SPDX-License-Identifier: GPL-3.0\r\n"
		"contract C {}\r\n";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_in_string)
{
	char const* sourceCode = R"(
		contract C {
			bytes license = "// SPDX-License-Identifier: GPL-3.0";
		}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == std::nullopt);
}

BOOST_AUTO_TEST_CASE(metadata_license_in_contract)
{
	char const* sourceCode = R"(
		contract C {
		// SPDX-License-Identifier: GPL-3.0
		}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == std::nullopt);
}

BOOST_AUTO_TEST_CASE(metadata_license_missing_colon)
{
	char const* sourceCode = R"(
		// SPDX-License-Identifier GPL-3.0
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == std::nullopt);
}

BOOST_AUTO_TEST_CASE(metadata_license_multiline)
{
	char const* sourceCode = R"(
		/* SPDX-License-Identifier: GPL-3.0 */
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_natspec)
{
	char const* sourceCode = R"(
		/// SPDX-License-Identifier: GPL-3.0
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_natspec_multiline)
{
	char const* sourceCode = R"(
		/** SPDX-License-Identifier: GPL-3.0 */
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_no_whitespace_multiline)
{
	char const* sourceCode = R"(
		/*SPDX-License-Identifier:GPL-3.0*/
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_nonempty_line)
{
	char const* sourceCode = R"(
		pragma solidity >= 0.0; // SPDX-License-Identifier: GPL-3.0
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_CASE(metadata_license_no_whitespace)
{
	char const* sourceCode = R"(
		//SPDX-License-Identifier:GPL-3.0
		contract C {}
	)";
	BOOST_CHECK(compileAndCheckLicenseMetadata("C", sourceCode) == "GPL-3.0");
}

BOOST_AUTO_TEST_SUITE_END()

}
