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
/** @file EVMSchedule.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 */

#pragma once

#include <libdevcore/Common.h>

namespace dev
{
namespace eth
{

struct EVMSchedule
{
	EVMSchedule(): tierStepGas(std::array<u256, 8>{{0, 2, 3, 5, 8, 10, 20, 0}}) {}
	EVMSchedule(bool _efcd, u256 const& _txCreateGas): exceptionalFailedCodeDeposit(_efcd), tierStepGas(std::array<u256, 8>{{0, 2, 3, 5, 8, 10, 20, 0}}), txCreateGas(_txCreateGas) {}
	bool exceptionalFailedCodeDeposit = true;
	u256 stackLimit = 1024;
	std::array<u256, 8> tierStepGas;
	u256 expGas = 10;
	u256 expByteGas = 10;
	u256 sha3Gas = 30;
	u256 sha3WordGas = 6;
	u256 sloadGas = 50;
	u256 sstoreSetGas = 20000;
	u256 sstoreResetGas = 5000;
	u256 sstoreRefundGas = 15000;
	u256 jumpdestGas = 1;
	u256 logGas = 375;
	u256 logDataGas = 8;
	u256 logTopicGas = 375;
	u256 createGas = 32000;
	u256 callGas = 40;
	u256 callStipend = 2300;
	u256 callValueTransferGas = 9000;
	u256 callNewAccountGas = 25000;
	u256 suicideRefundGas = 24000;
	u256 memoryGas = 3;
	u256 quadCoeffDiv = 512;
	u256 createDataGas = 200;
	u256 txGas = 21000;
	u256 txCreateGas = 53000;
	u256 txDataZeroGas = 4;
	u256 txDataNonZeroGas = 68;
	u256 copyGas = 3;
};

extern EVMSchedule const FrontierSchedule;
extern EVMSchedule const HomesteadSchedule;

}
}
