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
/** @file EVMSchedule.h
 * @author Gav <i@gavwood.com>
 * @author Christian <c@ethdev.com>
 * @date 2015
 */

#pragma once

namespace dev
{
namespace solidity
{

struct EVMSchedule
{
	unsigned stackLimit = 1024;
	unsigned expGas = 10;
	unsigned expByteGas = 10;
	unsigned sha3Gas = 30;
	unsigned sha3WordGas = 6;
	unsigned sloadGas = 200;
	unsigned sstoreSetGas = 20000;
	unsigned sstoreResetGas = 5000;
	unsigned sstoreRefundGas = 15000;
	unsigned jumpdestGas = 1;
	unsigned logGas = 375;
	unsigned logDataGas = 8;
	unsigned logTopicGas = 375;
	unsigned createGas = 32000;
	unsigned callGas = 40;
	unsigned callStipend = 2300;
	unsigned callValueTransferGas = 9000;
	unsigned callNewAccountGas = 25000;
	unsigned selfdestructRefundGas = 24000;
	unsigned memoryGas = 3;
	unsigned quadCoeffDiv = 512;
	unsigned createDataGas = 200;
	unsigned txGas = 21000;
	unsigned txCreateGas = 53000;
	unsigned txDataZeroGas = 4;
	unsigned txDataNonZeroGas = 68;
	unsigned copyGas = 3;	
};

}
}
