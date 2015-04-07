/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file createRandomStateTest.cpp
 * @author Christoph Jentzsch <jentzsch.simulationsoftware@gmail.com>
 * @date 2015
 * Creating a random state test.
 */

#include <string>
#include <iostream>
#include <chrono>

#include <boost/random.hpp>
#include <boost/filesystem/path.hpp>

#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <json_spirit/json_spirit.h>
#include <json_spirit/json_spirit_reader_template.h>
#include <json_spirit/json_spirit_writer_template.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonData.h>
#include <libevmcore/Instruction.h>
#include <libevm/VMFactory.h>
#include "TestHelper.h"
#include "vm.h"

using namespace std;
using namespace json_spirit;
using namespace dev;

void doStateTests(json_spirit::mValue& _v);

int main(int argc, char *argv[])
{
	g_logVerbosity = 0;

	// create random code

	boost::random::mt19937 gen;

	auto now = chrono::steady_clock::now().time_since_epoch();
	auto timeSinceEpoch = chrono::duration_cast<chrono::nanoseconds>(now).count();
	gen.seed(static_cast<unsigned int>(timeSinceEpoch));
	// set min and max length of the random evm code
	boost::random::uniform_int_distribution<> lengthOfCodeDist(8, 24);
	boost::random::uniform_int_distribution<> reasonableInputValuesSize(0, 7);
	boost::random::uniform_int_distribution<> opcodeDist(0, 255);
	boost::random::uniform_int_distribution<> BlockInfoOpcodeDist(0x40, 0x45);
	boost::random::uniform_int_distribution<> uniformInt(0, 0x7fffffff);
	boost::random::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<> > randGenInputValue(gen, reasonableInputValuesSize);
	boost::random::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<> > randGenUniformInt(gen, uniformInt);
	boost::random::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<> > randGen(gen, opcodeDist);
	boost::random::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<> > randGenBlockInfoOpcode(gen, BlockInfoOpcodeDist);

	std::vector<u256> reasonableInputValues;
	reasonableInputValues.push_back(0);
	reasonableInputValues.push_back(1);
	reasonableInputValues.push_back(50000);
	reasonableInputValues.push_back(u256("0x10000000000000000000000000000000000000000"));
	reasonableInputValues.push_back(u256("0xffffffffffffffffffffffffffffffffffffffff"));
	reasonableInputValues.push_back(u256("0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"));
	reasonableInputValues.push_back(u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
	reasonableInputValues.push_back(u256("0x945304eb96065b2a98b57a48a06ae28d285a71b5"));
	reasonableInputValues.push_back(randGenUniformInt());

	int lengthOfCode  = lengthOfCodeDist(gen);
	string randomCode;

	for (int i = 0; i < lengthOfCode; ++i)
	{
		// pre-fill stack to avoid that most of the test fail with a stackunderflow
		if (i < 8 && (randGen() < 192))
		{
			randomCode += randGen() < 32 ? toHex(toCompactBigEndian((uint8_t)randGenBlockInfoOpcode())) : "7f" +  toHex(reasonableInputValues[randGenInputValue()]);
			continue;
		}

		uint8_t opcode = randGen();
		// disregard all invalid commands, except of one (0x0c)
		if ((dev::eth::isValidInstruction(dev::eth::Instruction(opcode)) || (randGen() > 250)))
			randomCode += toHex(toCompactBigEndian(opcode));
		else
			i--;
	}

	string const s = R"(
	{
		"randomStatetest" : {
			"env" : {
				"currentCoinbase" : "945304eb96065b2a98b57a48a06ae28d285a71b5",
				"currentDifficulty" : "5623894562375",
				"currentGasLimit" : "115792089237316195423570985008687907853269984665640564039457584007913129639935",
				"currentNumber" : "0",
				"currentTimestamp" : "1",
				"previousHash" : "5e20a0453cecd065ea59c37ac63e079ee08998b6045136a8ce6635c7912ec0b6"
			},
			"pre" : {
				"095e7baea6a6c7c4c2dfeb977efac326af552d87" : {
					"balance" : "0",
					"code" : "0x6001600101600055",
					"nonce" : "0",
					"storage" : {
					}
				},
				"945304eb96065b2a98b57a48a06ae28d285a71b5" : {
					"balance" : "46",
					"code" : "0x6000355415600957005b60203560003555",
					"nonce" : "0",
					"storage" : {
					}
				},
				"a94f5374fce5edbc8e2a8697c15331677e6ebf0b" : {
					"balance" : "1000000000000000000",
					"code" : "0x",
					"nonce" : "0",
					"storage" : {
					}
				}
			},
			"transaction" : {
				"data" : "0x42",
				"gasLimit" : "400000",
				"gasPrice" : "1",
				"nonce" : "0",
				"secretKey" : "45a915e4d060149eb4365960e6a7a45f334393093061116b197e3240065ff2d8",
				"to" : "095e7baea6a6c7c4c2dfeb977efac326af552d87",
				"value" : "100000"
			}
		}
	}
)";
	mValue v;
	read_string(s, v);

	// insert new random code
	v.get_obj().find("randomStatetest")->second.get_obj().find("pre")->second.get_obj().begin()->second.get_obj()["code"] = "0x" + randomCode + (randGen() > 128 ? "55" : "") + (randGen() > 128 ? "60005155" : "");

	// insert new data in tx
	v.get_obj().find("randomStatetest")->second.get_obj().find("transaction")->second.get_obj()["data"] = "0x" + randomCode;

	// insert new value in tx
	v.get_obj().find("randomStatetest")->second.get_obj().find("transaction")->second.get_obj()["value"] = toString(randGenUniformInt());

	// insert new gasLimit in tx
	v.get_obj().find("randomStatetest")->second.get_obj().find("transaction")->second.get_obj()["gasLimit"] = "0x" + toHex(toCompactBigEndian((int)randGenUniformInt()));

	// fill test
	doStateTests(v);

	// stream to output for further handling by the bash script
	cout << json_spirit::write_string(v, true);

	return 0;
}

void doStateTests(json_spirit::mValue& _v)
{
	eth::VMFactory::setKind(eth::VMKind::Interpreter);

	for (auto& i: _v.get_obj())
	{
		//cerr << i.first << endl;
		mObject& o = i.second.get_obj();

		assert(o.count("env") > 0);
		assert(o.count("pre") > 0);
		assert(o.count("transaction") > 0);

		test::ImportTest importer(o, true);

		eth::State theState = importer.m_statePre;
		bytes output;

		try
		{
			output = theState.execute(test::lastHashes(importer.m_environment.currentBlock.number), importer.m_transaction).output;
		}
		catch (Exception const& _e)
		{
			cnote << "state execution did throw an exception: " << diagnostic_information(_e);
			theState.commit();
		}
		catch (std::exception const& _e)
		{
			cnote << "state execution did throw an exception: " << _e.what();
		}
#if ETH_FATDB
		importer.exportTest(output, theState);
#else
		cout << "You can not fill tests when FATDB is switched off";
#endif
	}
}

