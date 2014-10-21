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
/** @file createRandomTest.cpp
 * @author Christoph Jentzsch <jentzsch.simulationsoftware@gmail.com>
 * @date 2014
 * Creating a random virtual machine test.
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
#include <libevmface/Instruction.h>
#include "vm.h"

using namespace std;
using namespace json_spirit;
using namespace dev;

void doMyTests(json_spirit::mValue& v);

int main(int argc, char *argv[])
{
//	if (argc != 2)
//	{
//		cout << "usage: createRandomTest <filename>\n";
//		return 0;
//	}

	// create random code

	boost::random::mt19937 gen;

	auto now = chrono::steady_clock::now().time_since_epoch();
	auto timeSinceEpoch = chrono::duration_cast<chrono::nanoseconds>(now).count();
	gen.seed(static_cast<unsigned int>(timeSinceEpoch));
	boost::random::uniform_int_distribution<> lengthOfCodeDist(2, 16);
	boost::random::uniform_int_distribution<> opcodeDist(0, 255);
	boost::random::variate_generator<boost::mt19937&,
			boost::random::uniform_int_distribution<> > randGen(gen, opcodeDist);

	int lengthOfCode  = lengthOfCodeDist(gen);
	string randomCode;

	for (int i = 0; i < lengthOfCode; ++i)
	{
		uint8_t opcode = randGen();

		// disregard all invalid commands, except of one (0x10)
		if (dev::eth::isValidInstruction(dev::eth::Instruction(opcode)) || opcode == 0x10)
			randomCode += toHex(toCompactBigEndian(opcode));
		else
			i--;
	}

	const string s =\
"{\n\
	\"randomVMtest\": {\n\
		\"env\" : {\n\
			\"previousHash\" : \"5e20a0453cecd065ea59c37ac63e079ee08998b6045136a8ce6635c7912ec0b6\",\n\
			\"currentNumber\" : \"0\",\n\
			\"currentGasLimit\" : \"1000000\",\n\
			\"currentDifficulty\" : \"256\",\n\
			\"currentTimestamp\" : 1,\n\
			\"currentCoinbase\" : \"2adc25665018aa1fe0e6bc666dac8fc2697ff9ba\"\n\
		},\n\
		\"pre\" : {\n\
			\"0f572e5295c57f15886f9b263e2f6d2d6c7b5ec6\" : {\n\
				\"balance\" : \"1000000000000000000\",\n\
				\"nonce\" : 0,\n\
				\"code\" : \"random\",\n\
				\"storage\": {}\n\
			}\n\
		},\n\
		\"exec\" : {\n\
			\"address\" : \"0f572e5295c57f15886f9b263e2f6d2d6c7b5ec6\",\n\
			\"origin\" : \"cd1722f3947def4cf144679da39c4c32bdc35681\",\n\
			\"caller\" : \"cd1722f3947def4cf144679da39c4c32bdc35681\",\n\
			\"value\" : \"1000000000000000000\",\n\
			\"data\" : \"\",\n\
			\"gasPrice\" : \"100000000000000\",\n\
			\"gas\" : \"10000\"\n\
		}\n\
	}\n\
}";

	mValue v;
	read_string(s, v);

	// insert new random code
	v.get_obj().find("randomVMtest")->second.get_obj().find("pre")->second.get_obj().begin()->second.get_obj()["code"] = "0x" + randomCode;

	// execute code in vm
	doMyTests(v);

//	// write new test
//	string filename = argv[1];
//	writeFile(filename, asBytes(json_spirit::write_string(v, true)));

	// write resultsing test to envirnoment variable
	string str = "ETHEREUM_RANDOM_TEST=" + json_spirit::write_string(v, true);
	char *cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str());
	putenv(cstr);
	delete [] cstr;

	return 0;
}

void doMyTests(json_spirit::mValue& v)
{
	for (auto& i: v.get_obj())
	{
		mObject& o = i.second.get_obj();

		assert(o.count("env") > 0);
		assert(o.count("pre") > 0);
		assert(o.count("exec") > 0);

		eth::VM vm;
		test::FakeExtVM fev;
		fev.importEnv(o["env"].get_obj());
		fev.importState(o["pre"].get_obj());

		o["pre"] = mValue(fev.exportState());

		fev.importExec(o["exec"].get_obj());
		if (!fev.code)
		{
			fev.thisTxCode = get<3>(fev.addresses.at(fev.myAddress));
			fev.code = &fev.thisTxCode;
		}

		vm.reset(fev.gas);
		bytes output;
		try
		{
			output = vm.go(fev).toBytes();
		}
		catch (Exception const& _e)
		{
			cnote << "VM did throw an exception: " << diagnostic_information(_e);
		}
		catch (std::exception const& _e)
		{
			cnote << "VM did throw an exception: " << _e.what();
		}

		// delete null entries in storage for the sake of comparison

		for (auto  &a: fev.addresses)
		{
			vector<u256> keystoDelete;
			for (auto &s: get<2>(a.second))
			{
				if (s.second == 0)
					keystoDelete.push_back(s.first);
			}
			for (auto const key: keystoDelete )
			{
				get<2>(a.second).erase(key);
			}
		}

		o["env"] = mValue(fev.exportEnv());
		o["exec"] = mValue(fev.exportExec());
		o["post"] = mValue(fev.exportState());
		o["callcreates"] = fev.exportCallCreates();
		o["out"] = "0x" + toHex(output);
		fev.push(o, "gas", vm.gas());
	}
}
