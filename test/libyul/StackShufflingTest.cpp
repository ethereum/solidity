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

#include <test/libyul/StackShufflingTest.h>

#include <liblangutil/Scanner.h>
#include <libsolutil/AnsiColorized.h>
#include <libyul/backends/evm/StackHelpers.h>

using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace std;

bool StackShufflingTest::parse(string const& _source)
{
	CharStream stream(_source, "");
	Scanner scanner(stream);

	auto expectToken = [&](Token _token)
	{
		soltestAssert(
			scanner.next() == _token,
			"Invalid token. Expected: \"" + TokenTraits::friendlyName(_token) + "\"."
        );
	};

	auto parseStack = [&](Stack& stack) -> bool
	{
		if (scanner.currentToken() != Token::LBrack)
			return false;
		scanner.next();
		while (scanner.currentToken() != Token::RBrack &&
			   scanner.currentToken() != Token::EOS)
		{
			string literal = scanner.currentLiteral();
			if (literal == "RET")
			{
				scanner.next();
				if (scanner.currentToken() == Token::LBrack)
				{
					scanner.next();
					string functionName = scanner.currentLiteral();
					auto call = yul::FunctionCall{
						{},	yul::Identifier{{}, YulString(functionName)}, {}
					};
					stack.emplace_back(FunctionCallReturnLabelSlot{
							m_functions.insert(
								make_pair(functionName, call)
							).first->second
					});
					expectToken(Token::RBrack);
				}
				else
				{
					static Scope::Function function;
					stack.emplace_back(FunctionReturnLabelSlot{function});
					continue;
				}
			}
			else if (literal == "TMP")
			{
				expectToken(Token::LBrack);
				scanner.next();
				string functionName = scanner.currentLiteral();
				auto call = yul::FunctionCall{
				    {},	yul::Identifier{{}, YulString(functionName)}, {}
			    };
				expectToken(Token::Comma);
				scanner.next();
				size_t index = size_t(atoi(scanner.currentLiteral().c_str()));
				stack.emplace_back(TemporarySlot{
						m_functions.insert(make_pair(functionName, call)).first->second,
						index
				});
				expectToken(Token::RBrack);
			}
			else if (literal.find("0x") != string::npos || scanner.currentToken() == Token::Number)
			{
				stack.emplace_back(LiteralSlot{u256(literal)});
			}
			else if (literal == "JUNK")
			{
				stack.emplace_back(JunkSlot());
			}
			else if (literal == "GHOST")
			{
				expectToken(Token::LBrack);
				scanner.next(); // read number of ghost variables as ghostVariableId
				string ghostVariableId = scanner.currentLiteral();
				Scope::Variable ghostVar = Scope::Variable{""_yulstring, YulString(literal + "[" + ghostVariableId + "]")};
				stack.emplace_back(VariableSlot{
						m_variables.insert(make_pair(ghostVar.name, ghostVar)).first->second
				});
				expectToken(Token::RBrack);
			}
			else
			{
				Scope::Variable var = Scope::Variable{""_yulstring, YulString(literal)};
				stack.emplace_back(VariableSlot{
						m_variables.insert(
							make_pair(literal, var)
						).first->second
				});
			}
			scanner.next();
		}
		return scanner.currentToken() == Token::RBrack;
	};

	if (!parseStack(m_sourceStack))
		return false;
	scanner.next();
	return parseStack(m_targetStack);
}

StackShufflingTest::StackShufflingTest(string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult StackShufflingTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	if (!parse(m_source))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		return TestResult::FatalError;
	}

	ostringstream output;
	createStackLayout(
		m_sourceStack,
		m_targetStack,
		[&](unsigned _swapDepth) // swap
		{
			output << stackToString(m_sourceStack) << endl;
			output << "SWAP" << _swapDepth << endl;
		},
		[&](StackSlot const& _slot) // dupOrPush
		{
			output << stackToString(m_sourceStack) << endl;
			if (canBeFreelyGenerated(_slot))
				output << "PUSH " << stackSlotToString(_slot) << endl;
			else
			{
				if (auto depth = util::findOffset(m_sourceStack | ranges::views::reverse, _slot))
					output << "DUP" << *depth + 1 << endl;
				else
					BOOST_THROW_EXCEPTION(runtime_error("Invalid DUP operation."));
			}
		},
		[&](){ // pop
			output << stackToString(m_sourceStack) << endl;
			output << "POP" << endl;
		}
    );

	output << stackToString(m_sourceStack) << endl;
	m_obtainedResult = output.str();

	return checkResult(_stream, _linePrefix, _formatted);
}
