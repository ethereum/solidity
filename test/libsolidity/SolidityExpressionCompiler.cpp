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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Unit tests for the solidity expression compiler.
 */

#include <string>

#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/codegen/CompilerContext.h>
#include <libsolidity/codegen/ExpressionCompiler.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/interface/ErrorReporter.h>
#include <test/Options.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

/// Helper class that extracts the first expression in an AST.
class FirstExpressionExtractor: private ASTVisitor
{
public:
	FirstExpressionExtractor(ASTNode& _node): m_expression(nullptr) { _node.accept(*this); }
	Expression* expression() const { return m_expression; }
private:
	virtual bool visit(Assignment& _expression) override { return checkExpression(_expression); }
	virtual bool visit(UnaryOperation& _expression) override { return checkExpression(_expression); }
	virtual bool visit(BinaryOperation& _expression) override { return checkExpression(_expression); }
	virtual bool visit(FunctionCall& _expression) override { return checkExpression(_expression); }
	virtual bool visit(MemberAccess& _expression) override { return checkExpression(_expression); }
	virtual bool visit(IndexAccess& _expression) override { return checkExpression(_expression); }
	virtual bool visit(Identifier& _expression) override { return checkExpression(_expression); }
	virtual bool visit(ElementaryTypeNameExpression& _expression) override { return checkExpression(_expression); }
	virtual bool visit(Literal& _expression) override { return checkExpression(_expression); }
	bool checkExpression(Expression& _expression)
	{
		if (m_expression == nullptr)
			m_expression = &_expression;
		return false;
	}
private:
	Expression* m_expression;
};

Declaration const& resolveDeclaration(
	SourceUnit const& _sourceUnit,
	vector<string> const& _namespacedName,
	NameAndTypeResolver const& _resolver
)
{
	ASTNode const* scope = &_sourceUnit;
	// bracers are required, cause msvc couldn't handle this macro in for statement
	for (string const& namePart: _namespacedName)
	{
		auto declarations = _resolver.resolveName(namePart, scope);
		BOOST_REQUIRE(!declarations.empty());
		BOOST_REQUIRE(scope = *declarations.begin());
	}
	BOOST_REQUIRE(scope);
	return dynamic_cast<Declaration const&>(*scope);
}

bytes compileFirstExpression(
	const string& _sourceCode,
	vector<vector<string>> _functions = {},
	vector<vector<string>> _localVariables = {},
	vector<shared_ptr<MagicVariableDeclaration const>> _globalDeclarations = {}
)
{
	ASTPointer<SourceUnit> sourceUnit;
	try
	{
		ErrorList errors;
		ErrorReporter errorReporter(errors);
		sourceUnit = Parser(errorReporter).parse(make_shared<Scanner>(CharStream(_sourceCode)));
		if (!sourceUnit)
			return bytes();
	}
	catch(boost::exception const& _e)
	{
		auto msg = std::string("Parsing source code failed with: \n") + boost::diagnostic_information(_e);
		BOOST_FAIL(msg);
	}

	vector<Declaration const*> declarations;
	declarations.reserve(_globalDeclarations.size() + 1);
	for (ASTPointer<Declaration const> const& variable: _globalDeclarations)
		declarations.push_back(variable.get());

	ErrorList errors;
	ErrorReporter errorReporter(errors);
	map<ASTNode const*, shared_ptr<DeclarationContainer>> scopes;
	NameAndTypeResolver resolver(declarations, scopes, errorReporter);
	resolver.registerDeclarations(*sourceUnit);

	vector<ContractDefinition const*> inheritanceHierarchy;
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			BOOST_REQUIRE_MESSAGE(resolver.resolveNamesAndTypes(*contract), "Resolving names failed");
			inheritanceHierarchy = vector<ContractDefinition const*>(1, contract);
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			ErrorReporter errorReporter(errors);
			TypeChecker typeChecker(dev::test::Options::get().evmVersion(), errorReporter);
			BOOST_REQUIRE(typeChecker.checkTypeRequirements(*contract));
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			FirstExpressionExtractor extractor(*contract);
			BOOST_REQUIRE(extractor.expression() != nullptr);

			CompilerContext context(dev::test::Options::get().evmVersion());
			context.resetVisitedNodes(contract);
			context.setInheritanceHierarchy(inheritanceHierarchy);
			unsigned parametersSize = _localVariables.size(); // assume they are all one slot on the stack
			context.adjustStackOffset(parametersSize);
			for (vector<string> const& variable: _localVariables)
				context.addVariable(
					dynamic_cast<VariableDeclaration const&>(resolveDeclaration(*sourceUnit, variable, resolver)),
					parametersSize--
				);

			ExpressionCompiler(context).compile(*extractor.expression());

			for (vector<string> const& function: _functions)
				context << context.functionEntryLabel(dynamic_cast<FunctionDefinition const&>(
					resolveDeclaration(*sourceUnit, function, resolver)
				));
			bytes instructions = context.assembledObject().bytecode;
			// debug
			// cout << eth::disassemble(instructions) << endl;
			return instructions;
		}
	BOOST_FAIL("No contract found in source.");
	return bytes();
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(SolidityExpressionCompiler)

BOOST_AUTO_TEST_CASE(literal_true)
{
	char const* sourceCode = R"(
		contract test {
			function f() public { bool x = true; }
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH1), 0x1});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(literal_false)
{
	char const* sourceCode = R"(
		contract test {
			function f() { bool x = false; }
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH1), 0x0});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_literal)
{
	char const* sourceCode = R"(
		contract test {
		  function f() { uint x = 0x12345678901234567890; }
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH10), 0x12, 0x34, 0x56, 0x78, 0x90,
													   0x12, 0x34, 0x56, 0x78, 0x90});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_with_wei_ether_subdenomination)
{
	char const* sourceCode = R"(
		contract test {
			constructor() {
				 uint x = 1 wei;
			}
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH1), 0x1});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_with_szabo_ether_subdenomination)
{
	char const* sourceCode = R"(
		contract test {
			function test () {
				uint x = 1 szabo;
			}
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH5), 0xe8, 0xd4, 0xa5, 0x10, 0x00});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_with_finney_ether_subdenomination)
{
	char const* sourceCode = R"(
		contract test {
			constructor()
			{
				 uint x = 1 finney;
			}
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH7), 0x3, 0x8d, 0x7e, 0xa4, 0xc6, 0x80, 0x00});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_with_ether_ether_subdenomination)
{
	char const* sourceCode = R"(
		contract test {
			constructor() {
				 uint x = 1 ether;
			}
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH8), 0xd, 0xe0, 0xb6, 0xb3, 0xa7, 0x64, 0x00, 0x00});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(comparison)
{
	char const* sourceCode = R"(
		contract test {
			function f() { bool x = (0x10aa < 0x11aa) != true; }
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH1), 0x1, uint8_t(Instruction::ISZERO), uint8_t(Instruction::ISZERO),
					   uint8_t(Instruction::PUSH2), 0x11, 0xaa,
					   uint8_t(Instruction::PUSH2), 0x10, 0xaa,
					   uint8_t(Instruction::LT), uint8_t(Instruction::ISZERO), uint8_t(Instruction::ISZERO),
					   uint8_t(Instruction::EQ),
					   uint8_t(Instruction::ISZERO)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(short_circuiting)
{
	char const* sourceCode = R"(
		contract test {
			function f() { bool x = true != (4 <= 8 + 10 || 9 != 2); }
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({uint8_t(Instruction::PUSH1), 0x12, // 8 + 10
					   uint8_t(Instruction::PUSH1), 0x4,
					   uint8_t(Instruction::GT),
					   uint8_t(Instruction::ISZERO), // after this we have 4 <= 8 + 10
					   uint8_t(Instruction::DUP1),
					   uint8_t(Instruction::PUSH1), 0x11,
					   uint8_t(Instruction::JUMPI), // short-circuit if it is true
					   uint8_t(Instruction::POP),
					   uint8_t(Instruction::PUSH1), 0x2,
					   uint8_t(Instruction::PUSH1), 0x9,
					   uint8_t(Instruction::EQ),
					   uint8_t(Instruction::ISZERO), // after this we have 9 != 2
					   uint8_t(Instruction::JUMPDEST),
					   uint8_t(Instruction::ISZERO), uint8_t(Instruction::ISZERO),
					   uint8_t(Instruction::PUSH1), 0x1, uint8_t(Instruction::ISZERO), uint8_t(Instruction::ISZERO),
					   uint8_t(Instruction::EQ),
					   uint8_t(Instruction::ISZERO)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(arithmetic)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint y) { ((((((((y ^ 8) & 7) | 6) - 5) + 4) % 3) / 2) * 1); }
		}
	)";
	bytes code = compileFirstExpression(sourceCode, {}, {{"test", "f", "y"}});
	bytes expectation({uint8_t(Instruction::PUSH1), 0x1,
					   uint8_t(Instruction::PUSH1), 0x2,
					   uint8_t(Instruction::PUSH1), 0x3,
					   uint8_t(Instruction::PUSH1), 0x4,
					   uint8_t(Instruction::PUSH1), 0x5,
					   uint8_t(Instruction::PUSH1), 0x6,
					   uint8_t(Instruction::PUSH1), 0x7,
					   uint8_t(Instruction::PUSH1), 0x8,
					   uint8_t(Instruction::DUP9),
					   uint8_t(Instruction::XOR),
					   uint8_t(Instruction::AND),
					   uint8_t(Instruction::OR),
					   uint8_t(Instruction::SUB),
					   uint8_t(Instruction::ADD),
					   uint8_t(Instruction::DUP2),
					   uint8_t(Instruction::ISZERO),
					   uint8_t(Instruction::ISZERO),
					   uint8_t(Instruction::PUSH1), 0x1d,
					   uint8_t(Instruction::JUMPI),
					   uint8_t(Instruction::INVALID),
					   uint8_t(Instruction::JUMPDEST),
					   uint8_t(Instruction::MOD),
					   uint8_t(Instruction::DUP2),
					   uint8_t(Instruction::ISZERO),
					   uint8_t(Instruction::ISZERO),
					   uint8_t(Instruction::PUSH1), 0x26,
					   uint8_t(Instruction::JUMPI),
					   uint8_t(Instruction::INVALID),
					   uint8_t(Instruction::JUMPDEST),
					   uint8_t(Instruction::DIV),
					   uint8_t(Instruction::MUL)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(unary_operators)
{
	char const* sourceCode = R"(
		contract test {
			function f(int y) { !(~+- y == 2); }
		}
	)";
	bytes code = compileFirstExpression(sourceCode, {}, {{"test", "f", "y"}});

	bytes expectation({uint8_t(Instruction::PUSH1), 0x2,
					   uint8_t(Instruction::DUP2),
					   uint8_t(Instruction::PUSH1), 0x0,
					   uint8_t(Instruction::SUB),
					   uint8_t(Instruction::NOT),
					   uint8_t(Instruction::EQ),
					   uint8_t(Instruction::ISZERO)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(unary_inc_dec)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns (uint x) { x = --a ^ (a-- ^ (++a ^ a++)); }
		}
	)";
	bytes code = compileFirstExpression(sourceCode, {}, {{"test", "f", "a"}, {"test", "f", "x"}});

	// Stack: a, x
	bytes expectation({uint8_t(Instruction::DUP2),
					   uint8_t(Instruction::DUP1),
					   uint8_t(Instruction::PUSH1), 0x1,
					   uint8_t(Instruction::ADD),
					   // Stack here: a x a (a+1)
					   uint8_t(Instruction::SWAP3),
					   uint8_t(Instruction::POP), // first ++
					   // Stack here: (a+1) x a
					   uint8_t(Instruction::DUP3),
					   uint8_t(Instruction::PUSH1), 0x1,
					   uint8_t(Instruction::ADD),
					   // Stack here: (a+1) x a (a+2)
					   uint8_t(Instruction::SWAP3),
					   uint8_t(Instruction::POP),
					   // Stack here: (a+2) x a
					   uint8_t(Instruction::DUP3), // second ++
					   uint8_t(Instruction::XOR),
					   // Stack here: (a+2) x a^(a+2)
					   uint8_t(Instruction::DUP3),
					   uint8_t(Instruction::DUP1),
					   uint8_t(Instruction::PUSH1), 0x1,
					   uint8_t(Instruction::SWAP1),
					   uint8_t(Instruction::SUB),
					   // Stack here: (a+2) x a^(a+2) (a+2) (a+1)
					   uint8_t(Instruction::SWAP4),
					   uint8_t(Instruction::POP), // first --
					   uint8_t(Instruction::XOR),
					   // Stack here: (a+1) x a^(a+2)^(a+2)
					   uint8_t(Instruction::DUP3),
					   uint8_t(Instruction::PUSH1), 0x1,
					   uint8_t(Instruction::SWAP1),
					   uint8_t(Instruction::SUB),
					   // Stack here: (a+1) x a^(a+2)^(a+2) a
					   uint8_t(Instruction::SWAP3),
					   uint8_t(Instruction::POP), // second ++
					   // Stack here: a x a^(a+2)^(a+2)
					   uint8_t(Instruction::DUP3), // will change
					   uint8_t(Instruction::XOR),
					   uint8_t(Instruction::SWAP1),
					   uint8_t(Instruction::POP),
					   uint8_t(Instruction::DUP1)});
					   // Stack here: a x a^(a+2)^(a+2)^a
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(assignment)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a, uint b) { (a += b) * 2; }
		}
	)";
	bytes code = compileFirstExpression(sourceCode, {}, {{"test", "f", "a"}, {"test", "f", "b"}});

	// Stack: a, b
	bytes expectation({uint8_t(Instruction::PUSH1), 0x2,
					   uint8_t(Instruction::DUP2),
					   uint8_t(Instruction::DUP4),
					   uint8_t(Instruction::ADD),
					   // Stack here: a b 2 a+b
					   uint8_t(Instruction::SWAP3),
					   uint8_t(Instruction::POP),
					   uint8_t(Instruction::DUP3),
					   // Stack here: a+b b 2 a+b
					   uint8_t(Instruction::MUL)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(negative_literals_8bits)
{
	char const* sourceCode = R"(
		contract test {
			function f() { int8 x = -0x80; }
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation(bytes({uint8_t(Instruction::PUSH32)}) + bytes(31, 0xff) + bytes(1, 0x80));
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(negative_literals_16bits)
{
	char const* sourceCode = R"(
		contract test {
			function f() { int64 x = ~0xabc; }
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation(bytes({uint8_t(Instruction::PUSH32)}) + bytes(30, 0xff) + bytes{0xf5, 0x43});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(intermediately_overflowing_literals)
{
	// first literal itself is too large for 256 bits but it fits after all constant operations
	// have been applied
	char const* sourceCode = R"(
		contract test {
			function f() { uint8 x = (0x00ffffffffffffffffffffffffffffffffffffffff * 0xffffffffffffffffffffffffff01) & 0xbf; }
		}
	)";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation(bytes({uint8_t(Instruction::PUSH1), 0xbf}));
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(blockhash)
{
	char const* sourceCode = R"(
		contract test {
			function f() {
				blockhash(3);
			}
		}
	)";

	auto blockhashFun = make_shared<FunctionType>(strings{"uint256"}, strings{"bytes32"},
		FunctionType::Kind::BlockHash, false, StateMutability::View);

	bytes code = compileFirstExpression(sourceCode, {}, {}, {make_shared<MagicVariableDeclaration>("blockhash", blockhashFun)});

	bytes expectation({uint8_t(Instruction::PUSH1), 0x03,
					   uint8_t(Instruction::BLOCKHASH)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(gas_left)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns (uint256 val) {
				return gasleft();
			}
		}
	)";
	bytes code = compileFirstExpression(
		sourceCode, {}, {},
		{make_shared<MagicVariableDeclaration>("gasleft", make_shared<FunctionType>(strings(), strings{"uint256"}, FunctionType::Kind::GasLeft))}
	);

	bytes expectation = bytes({uint8_t(Instruction::GAS)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
