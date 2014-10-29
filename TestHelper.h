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
/** @file TestHelper.h
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 */

#pragma once

#include "JsonSpiritHeaders.h"
#include <libethereum/State.h>

namespace dev
{
namespace eth
{

class Client;

void mine(Client& c, int numBlocks);
void connectClients(Client& c1, Client& c2);

namespace test
{

class ImportTest
{
public:
	ImportTest() = default;
	ImportTest(json_spirit::mObject& _o, bool isFiller);

	// imports
	void importEnv(json_spirit::mObject& _o);
	void importState(json_spirit::mObject& _o, State& _state);
	void importExec(json_spirit::mObject& _o);
	void importCallCreates(json_spirit::mArray& _callcreates);
	void importGas(json_spirit::mObject& _o);
	void importOutput(json_spirit::mObject& _o);

	void exportTest();
	Manifest* getManifest(){ return &m_manifest;}

	State m_statePre;
	State m_statePost;
	ExtVMFace m_environment;
	u256 gas;
	u256 gasExec;
	Transactions callcreates;
	bytes output;
	Manifest m_manifest;

private:
	// needed for const refs
	bytes code;
	bytes data;
};

// helping functions

u256 toInt(json_spirit::mValue const& _v);
byte toByte(json_spirit::mValue const& _v);

}
}
}
