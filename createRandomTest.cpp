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
#include <boost/random.hpp>
#include <boost/chrono.hpp>
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
	if (argc != 2)
	{
		cout << "usage: createRandomTest <filename>\n";
		return 0;
	}

	// create random code

	boost::random::mt19937 gen;

	auto now = boost::chrono::steady_clock::now().time_since_epoch();
	auto timeSinceEpoch = boost::chrono::duration_cast<boost::chrono::nanoseconds>(now).count();
	gen.seed(static_cast<unsigned int>(timeSinceEpoch));
	boost::random::uniform_int_distribution<> lengthOfCodeDist(2, 16);
	boost::random::uniform_int_distribution<> opcodeDist(0, 255);
	boost::random::variate_generator<boost::mt19937&,
			boost::random::uniform_int_distribution<> > randGen(gen, opcodeDist);

	int lengthOfCode  = lengthOfCodeDist(gen);
	string randomCode;

	for (int i = 0; i < lengthOfCode; ++i)
	{
		randomCode += toHex(toCompactBigEndian(randGen()));
	}

	// read template test file

	mValue v;
	boost::filesystem::path p(__FILE__);
	boost::filesystem::path dir = p.parent_path();
	string s = asString(contents(dir.string() + "/randomTestFiller.json"));
	read_string(s, v);

	// insert new random code
	v.get_obj().find("randomVMtest")->second.get_obj().find("pre")->second.get_obj().begin()->second.get_obj()["code"] = "0x" + randomCode;

	// execute code in vm
	doMyTests(v);

	// write new test
	string filename = argv[1];
	writeFile(filename, asBytes(json_spirit::write_string(v, true)));

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
