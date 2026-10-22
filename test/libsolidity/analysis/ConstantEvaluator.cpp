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
 * Unit tests for the constant evaluator.
 */

#include <test/libsolidity/AnalysisFramework.h>

#include <test/Common.h>

#include <libsolidity/analysis/ConstantEvaluator.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libsolutil/CommonData.h>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace solidity::langutil;
using namespace solidity::util;

namespace solidity::frontend::test
{

namespace
{

class ConstantEvaluatorFramework: public AnalysisFramework
{
protected:
	/// Evaluates the initializer of the file-level constant `probe`. The initializer is expected
	/// to reference the constant under test, so that its value goes through the conversion
	/// to the declared type of that constant.
	ConstantEvaluator::TypedValue evaluateProbe()
	{
		BOOST_REQUIRE(stageSuccessful(PipelineStage::Parsing));
		VariableDeclaration const* probe = nullptr;
		for (auto const* variable: ASTNode::filteredNodes<VariableDeclaration>(compiler().ast("").nodes()))
			if (variable->name() == "probe")
				probe = variable;
		BOOST_REQUIRE(probe && probe->value());
		return ConstantEvaluator::tryEvaluate(*probe->value());
	}

	void checkBytes32(std::string const& _source, std::string const& _expectedHex)
	{
		BOOST_REQUIRE(runFramework(_source, PipelineStage::Analysis));
		ConstantEvaluator::TypedValue value = evaluateProbe();
		BOOST_REQUIRE(value.isBytes());
		BOOST_CHECK(&value.type() == TypeProvider::fixedBytes(32));
		BOOST_CHECK_EQUAL(toHex(fromBigEndian<u256>(value.asBytes())), _expectedHex);
	}

	/// For sources rejected by the type checker. The evaluator still runs on them
	/// from DeclarationTypeChecker, so it has to decline the conversion on its own.
	void checkRejected(std::string const& _source)
	{
		BOOST_REQUIRE(!runFramework(_source, PipelineStage::Analysis));
		BOOST_CHECK(evaluateProbe().isEmpty());
	}

	void checkUnsupported(std::string const& _source)
	{
		BOOST_REQUIRE(runFramework(_source, PipelineStage::Analysis));
		BOOST_CHECK(evaluateProbe().isEmpty());
	}
};

}

BOOST_FIXTURE_TEST_SUITE(ConstantEvaluatorTest, ConstantEvaluatorFramework)

BOOST_AUTO_TEST_CASE(string_literal_to_bytes32_right_padded)
{
	checkBytes32(
		"bytes32 constant X = \"abc\"; bytes32 constant probe = X;",
		"616263" + std::string(58, '0')
	);
}

BOOST_AUTO_TEST_CASE(hex_string_literal_to_bytes32_right_padded)
{
	checkBytes32(
		"bytes32 constant X = hex\"4d41\"; bytes32 constant probe = X;",
		"4d41" + std::string(60, '0')
	);
}

BOOST_AUTO_TEST_CASE(empty_string_literal_to_bytes32)
{
	checkBytes32(
		"bytes32 constant X = \"\"; bytes32 constant probe = X;",
		std::string(64, '0')
	);
}

BOOST_AUTO_TEST_CASE(full_width_string_literal_to_bytes32)
{
	checkBytes32(
		"bytes32 constant X = \"abcdefghijklmnopqrstuvwxyz012345\"; bytes32 constant probe = X;",
		"6162636465666768696a6b6c6d6e6f707172737475767778797a303132333435"
	);
}

BOOST_AUTO_TEST_CASE(hex_number_to_bytes32_big_endian)
{
	checkBytes32(
		"bytes32 constant X = 0x0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20;"
		"bytes32 constant probe = X;",
		"0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20"
	);
}

BOOST_AUTO_TEST_CASE(max_hex_number_to_bytes32)
{
	checkBytes32(
		"bytes32 constant X = 0x" + std::string(64, 'f') + "; bytes32 constant probe = X;",
		std::string(64, 'f')
	);
}

BOOST_AUTO_TEST_CASE(zero_to_bytes32)
{
	checkBytes32(
		"bytes32 constant X = 0; bytes32 constant probe = X;",
		std::string(64, '0')
	);
}

BOOST_AUTO_TEST_CASE(keccak256_constant_to_bytes32)
{
	checkBytes32(
	"bytes32 constant A = keccak256(\"x\"); bytes32 constant probe = A;",
	"7521d1cadbcfa91eec65aa16715b94ffc1c9654ba57ea2ef1a2127bca1127a83"
	);
}

BOOST_AUTO_TEST_CASE(string_literal_longer_than_bytes32_rejected)
{
	checkRejected("bytes32 constant X = \"abcdefghijklmnopqrstuvwxyz0123456\"; bytes32 constant probe = X;");
}

BOOST_AUTO_TEST_CASE(negative_rational_to_bytes32_rejected)
{
	checkRejected("bytes32 constant X = -1; bytes32 constant probe = X;");
}

BOOST_AUTO_TEST_CASE(fractional_rational_to_bytes32_rejected)
{
	checkRejected("bytes32 constant X = 1.5; bytes32 constant probe = X;");
}

BOOST_AUTO_TEST_CASE(rational_above_bytes32_max_rejected)
{
	checkRejected("bytes32 constant X = 0x1" + std::string(64, '0') + "; bytes32 constant probe = X;");
}

BOOST_AUTO_TEST_CASE(narrower_fixed_bytes_unsupported)
{
	checkUnsupported("bytes4 constant X = \"abcd\"; bytes4 constant probe = X;");
	checkUnsupported("bytes4 constant X = 0x01020304; bytes4 constant probe = X;");
}

BOOST_AUTO_TEST_SUITE_END()

}
