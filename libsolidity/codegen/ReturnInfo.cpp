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
			if (retType->isDynamicallyEncoded())
			{
				solAssert(haveReturndatacopy, "");
				dynamicReturnSize = true;
				estimatedReturnSize = 0;
				break;
			}
			else if (retType->decodingType())
				estimatedReturnSize += retType->decodingType()->calldataEncodedSize();
			else
				estimatedReturnSize += retType->calldataEncodedSize();
	}
}
