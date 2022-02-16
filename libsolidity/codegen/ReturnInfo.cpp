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

#include <libsolidity/codegen/ReturnInfo.h>

#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/AST.h>

using namespace solidity::frontend;
using namespace solidity::langutil;

ReturnInfo::ReturnInfo(EVMVersion const& _evmVersion, FunctionType const& _functionType)
{
	FunctionType::Kind const funKind = _functionType.kind();
	bool const haveReturndatacopy = _evmVersion.supportsReturndata();
	bool const returnSuccessConditionAndReturndata =
		funKind == FunctionType::Kind::BareCall ||
		funKind == FunctionType::Kind::BareDelegateCall ||
		funKind == FunctionType::Kind::BareStaticCall;

	if (!returnSuccessConditionAndReturndata)
	{
		if (haveReturndatacopy)
			returnTypes = _functionType.returnParameterTypes();
		else
			returnTypes = _functionType.returnParameterTypesWithoutDynamicTypes();

		for (auto const& retType: returnTypes)
		{
			solAssert(retType->decodingType(), "");
			if (retType->decodingType()->isDynamicallyEncoded())
			{
				solAssert(haveReturndatacopy, "");
				dynamicReturnSize = true;
				estimatedReturnSize = 0;
				break;
			}
			else
				estimatedReturnSize += retType->decodingType()->calldataEncodedSize();
		}
	}
	if (dynamicReturnSize)
		solAssert(estimatedReturnSize == 0);
}
