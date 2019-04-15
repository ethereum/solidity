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
 * @author Christian <chris@ethereum.org>
 * @date 2017
 * Routines that generate Yul code related to ABI encoding, decoding and type conversions.
 */

#include <libsolidity/codegen/ABIFunctions.h>

#include <libsolidity/codegen/CompilerUtils.h>
#include <libdevcore/Whiskers.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

string ABIFunctions::tupleEncoder(
	TypePointers const& _givenTypes,
	TypePointers const& _targetTypes,
	bool _encodeAsLibraryTypes
)
{
	EncodingOptions options;
	options.encodeAsLibraryTypes = _encodeAsLibraryTypes;
	options.encodeFunctionFromStack = true;
	options.padded = true;
	options.dynamicInplace = false;

	string functionName = string("abi_encode_tuple_");
	for (auto const& t: _givenTypes)
		functionName += t->identifier() + "_";
	functionName += "_to_";
	for (auto const& t: _targetTypes)
		functionName += t->identifier() + "_";
	functionName += options.toFunctionNameSuffix();

	return createExternallyUsedFunction(functionName, [&]() {
		// Note that the values are in reverse due to the difference in calling semantics.
		Whiskers templ(R"(
			function <functionName>(headStart <valueParams>) -> tail {
				tail := add(headStart, <headSize>)
				<encodeElements>
			}
		)");
		templ("functionName", functionName);
		size_t const headSize_ = headSize(_targetTypes);
		templ("headSize", to_string(headSize_));
		string encodeElements;
		size_t headPos = 0;
		size_t stackPos = 0;
		for (size_t i = 0; i < _givenTypes.size(); ++i)
		{
			solAssert(_givenTypes[i], "");
			solAssert(_targetTypes[i], "");
			size_t sizeOnStack = _givenTypes[i]->sizeOnStack();
			bool dynamic = _targetTypes[i]->isDynamicallyEncoded();
			Whiskers elementTempl(
				dynamic ?
				string(R"(
					mstore(add(headStart, <pos>), sub(tail, headStart))
					tail := <abiEncode>(<values> tail)
				)") :
				string(R"(
					<abiEncode>(<values> add(headStart, <pos>))
				)")
			);
			string values = m_utils.suffixedVariableNameList("value", stackPos, stackPos + sizeOnStack);
			elementTempl("values", values.empty() ? "" : values + ", ");
			elementTempl("pos", to_string(headPos));
			elementTempl("abiEncode", abiEncodingFunction(*_givenTypes[i], *_targetTypes[i], options));
			encodeElements += elementTempl.render();
			headPos += dynamic ? 0x20 : _targetTypes[i]->calldataEncodedSize();
			stackPos += sizeOnStack;
		}
		solAssert(headPos == headSize_, "");
		string valueParams = m_utils.suffixedVariableNameList("value", stackPos, 0);
		templ("valueParams", valueParams.empty() ? "" : ", " + valueParams);
		templ("encodeElements", encodeElements);

		return templ.render();
	});
}

string ABIFunctions::tupleEncoderPacked(
	TypePointers const& _givenTypes,
	TypePointers const& _targetTypes
)
{
	EncodingOptions options;
	options.encodeAsLibraryTypes = false;
	options.encodeFunctionFromStack = true;
	options.padded = false;
	options.dynamicInplace = true;

	string functionName = string("abi_encode_tuple_packed_");
	for (auto const& t: _givenTypes)
		functionName += t->identifier() + "_";
	functionName += "_to_";
	for (auto const& t: _targetTypes)
		functionName += t->identifier() + "_";
	functionName += options.toFunctionNameSuffix();

	return createExternallyUsedFunction(functionName, [&]() {
		solAssert(!_givenTypes.empty(), "");

		// Note that the values are in reverse due to the difference in calling semantics.
		Whiskers templ(R"(
			function <functionName>(pos <valueParams>) -> end {
				<encodeElements>
				end := pos
			}
		)");
		templ("functionName", functionName);
		string encodeElements;
		size_t stackPos = 0;
		for (size_t i = 0; i < _givenTypes.size(); ++i)
		{
			solAssert(_givenTypes[i], "");
			solAssert(_targetTypes[i], "");
			size_t sizeOnStack = _givenTypes[i]->sizeOnStack();
			bool dynamic = _targetTypes[i]->isDynamicallyEncoded();
			Whiskers elementTempl(
				dynamic ?
				string(R"(
					pos := <abiEncode>(<values> pos)
				)") :
				string(R"(
					<abiEncode>(<values> pos)
					pos := add(pos, <calldataEncodedSize>)
				)")
			);
			string values = m_utils.suffixedVariableNameList("value", stackPos, stackPos + sizeOnStack);
			elementTempl("values", values.empty() ? "" : values + ", ");
			if (!dynamic)
				elementTempl("calldataEncodedSize", to_string(_targetTypes[i]->calldataEncodedSize(false)));
			elementTempl("abiEncode", abiEncodingFunction(*_givenTypes[i], *_targetTypes[i], options));
			encodeElements += elementTempl.render();
			stackPos += sizeOnStack;
		}
		string valueParams = m_utils.suffixedVariableNameList("value", stackPos, 0);
		templ("valueParams", valueParams.empty() ? "" : ", " + valueParams);
		templ("encodeElements", encodeElements);

		return templ.render();
	});
}
string ABIFunctions::tupleDecoder(TypePointers const& _types, bool _fromMemory)
{
	string functionName = string("abi_decode_tuple_");
	for (auto const& t: _types)
		functionName += t->identifier();
	if (_fromMemory)
		functionName += "_fromMemory";

	return createExternallyUsedFunction(functionName, [&]() {
		TypePointers decodingTypes;
		for (auto const& t: _types)
			decodingTypes.emplace_back(t->decodingType());

		Whiskers templ(R"(
			function <functionName>(headStart, dataEnd) <arrow> <valueReturnParams> {
				if slt(sub(dataEnd, headStart), <minimumSize>) { revert(0, 0) }
				<decodeElements>
			}
		)");
		templ("functionName", functionName);
		templ("minimumSize", to_string(headSize(decodingTypes)));

		string decodeElements;
		vector<string> valueReturnParams;
		size_t headPos = 0;
		size_t stackPos = 0;
		for (size_t i = 0; i < _types.size(); ++i)
		{
			solAssert(_types[i], "");
			solAssert(decodingTypes[i], "");
			size_t sizeOnStack = _types[i]->sizeOnStack();
			solAssert(sizeOnStack == decodingTypes[i]->sizeOnStack(), "");
			solAssert(sizeOnStack > 0, "");
			vector<string> valueNamesLocal;
			for (size_t j = 0; j < sizeOnStack; j++)
			{
				valueNamesLocal.emplace_back("value" + to_string(stackPos));
				valueReturnParams.emplace_back("value" + to_string(stackPos));
				stackPos++;
			}
			bool dynamic = decodingTypes[i]->isDynamicallyEncoded();
			Whiskers elementTempl(
				dynamic ?
				R"(
				{
					let offset := <load>(add(headStart, <pos>))
					if gt(offset, 0xffffffffffffffff) { revert(0, 0) }
					<values> := <abiDecode>(add(headStart, offset), dataEnd)
				}
				)" :
				R"(
				{
					let offset := <pos>
					<values> := <abiDecode>(add(headStart, offset), dataEnd)
				}
				)"
			);
			elementTempl("load", _fromMemory ? "mload" : "calldataload");
			elementTempl("values", boost::algorithm::join(valueNamesLocal, ", "));
			elementTempl("pos", to_string(headPos));
			elementTempl("abiDecode", abiDecodingFunction(*_types[i], _fromMemory, true));
			decodeElements += elementTempl.render();
			headPos += dynamic ? 0x20 : decodingTypes[i]->calldataEncodedSize();
		}
		templ("valueReturnParams", boost::algorithm::join(valueReturnParams, ", "));
		templ("arrow", valueReturnParams.empty() ? "" : "->");
		templ("decodeElements", decodeElements);

		return templ.render();
	});
}

pair<string, set<string>> ABIFunctions::requestedFunctions()
{
	std::set<string> empty;
	swap(empty, m_externallyUsedFunctions);
	return make_pair(m_functionCollector->requestedFunctions(), std::move(empty));
}

string ABIFunctions::EncodingOptions::toFunctionNameSuffix() const
{
	string suffix;
	if (!padded)
		suffix += "_nonPadded";
	if (dynamicInplace)
		suffix += "_inplace";
	if (encodeFunctionFromStack)
		suffix += "_fromStack";
	if (encodeAsLibraryTypes)
		suffix += "_library";
	return suffix;
}

string ABIFunctions::cleanupFromStorageFunction(Type const& _type, bool _splitFunctionTypes)
{
	solAssert(_type.isValueType(), "");
	solUnimplementedAssert(!_splitFunctionTypes, "");

	string functionName = string("cleanup_from_storage_") + (_splitFunctionTypes ? "split_" : "") + _type.identifier();
	return createFunction(functionName, [&] {
		Whiskers templ(R"(
			function <functionName>(value) -> cleaned {
				<body>
			}
		)");
		templ("functionName", functionName);

		unsigned storageBytes = _type.storageBytes();
		if (IntegerType const* type = dynamic_cast<IntegerType const*>(&_type))
			if (type->isSigned() && storageBytes != 32)
			{
				templ("body", "cleaned := signextend(" + to_string(storageBytes - 1) + ", value)");
				return templ.render();
			}

		if (storageBytes == 32)
			templ("body", "cleaned := value");
		else if (_type.leftAligned())
			templ("body", "cleaned := " + m_utils.shiftLeftFunction(256 - 8 * storageBytes) + "(value)");
		else
			templ("body", "cleaned := and(value, " + toCompactHexWithPrefix((u256(1) << (8 * storageBytes)) - 1) + ")");

		return templ.render();
	});
}

string ABIFunctions::abiEncodingFunction(
	Type const& _from,
	Type const& _to,
	EncodingOptions const& _options
)
{
	TypePointer toInterface = _to.fullEncodingType(_options.encodeAsLibraryTypes, true, false);
	solUnimplementedAssert(toInterface, "Encoding type \"" + _to.toString() + "\" not yet implemented.");
	Type const& to = *toInterface;

	if (_from.category() == Type::Category::StringLiteral)
		return abiEncodingFunctionStringLiteral(_from, to, _options);
	else if (auto toArray = dynamic_cast<ArrayType const*>(&to))
	{
		solAssert(_from.category() == Type::Category::Array, "");
		solAssert(to.dataStoredIn(DataLocation::Memory), "");
		ArrayType const& fromArray = dynamic_cast<ArrayType const&>(_from);

		switch (fromArray.location())
		{
			case DataLocation::CallData:
				if (
					fromArray.isByteArray() ||
					*fromArray.baseType() == *TypeProvider::integerType() ||
					*fromArray.baseType() == FixedBytesType(32)
				)
					return abiEncodingFunctionCalldataArrayWithoutCleanup(fromArray, *toArray, _options);
				else
					return abiEncodingFunctionSimpleArray(fromArray, *toArray, _options);
			case DataLocation::Memory:
				if (fromArray.isByteArray())
					return abiEncodingFunctionMemoryByteArray(fromArray, *toArray, _options);
				else
					return abiEncodingFunctionSimpleArray(fromArray, *toArray, _options);
			case DataLocation::Storage:
				if (fromArray.baseType()->storageBytes() <= 16)
					return abiEncodingFunctionCompactStorageArray(fromArray, *toArray, _options);
				else
					return abiEncodingFunctionSimpleArray(fromArray, *toArray, _options);
			default:
				solAssert(false, "");
		}
	}
	else if (auto const* toStruct = dynamic_cast<StructType const*>(&to))
	{
		StructType const* fromStruct = dynamic_cast<StructType const*>(&_from);
		solAssert(fromStruct, "");
		return abiEncodingFunctionStruct(*fromStruct, *toStruct, _options);
	}
	else if (_from.category() == Type::Category::Function)
		return abiEncodingFunctionFunctionType(
			dynamic_cast<FunctionType const&>(_from),
			to,
			_options
		);

	solAssert(_from.sizeOnStack() == 1, "");
	solAssert(to.isValueType(), "");
	solAssert(to.calldataEncodedSize() == 32, "");
	string functionName =
		"abi_encode_" +
		_from.identifier() +
		"_to_" +
		to.identifier() +
		_options.toFunctionNameSuffix();
	return createFunction(functionName, [&]() {
		solAssert(!to.isDynamicallyEncoded(), "");

		Whiskers templ(R"(
			function <functionName>(value, pos) {
				mstore(pos, <cleanupConvert>)
			}
		)");
		templ("functionName", functionName);

		if (_from.dataStoredIn(DataLocation::Storage))
		{
			// special case: convert storage reference type to value type - this is only
			// possible for library calls where we just forward the storage reference
			solAssert(_options.encodeAsLibraryTypes, "");
			solAssert(_options.padded && !_options.dynamicInplace, "Non-padded / inplace encoding for library call requested.");
			solAssert(to == *TypeProvider::integerType(), "");
			templ("cleanupConvert", "value");
		}
		else
		{
			string cleanupConvert;
			if (_from == to)
				cleanupConvert = m_utils.cleanupFunction(_from) + "(value)";
			else
				cleanupConvert = m_utils.conversionFunction(_from, to) + "(value)";
			if (!_options.padded)
				cleanupConvert = m_utils.leftAlignFunction(to) + "(" + cleanupConvert + ")";
			templ("cleanupConvert", cleanupConvert);
		}
		return templ.render();
	});
}

string ABIFunctions::abiEncodeAndReturnUpdatedPosFunction(
	Type const& _givenType,
	Type const& _targetType,
	ABIFunctions::EncodingOptions const& _options
)
{
	string functionName =
		"abi_encodeUpdatedPos_" +
		_givenType.identifier() +
		"_to_" +
		_targetType.identifier() +
		_options.toFunctionNameSuffix();
	return createFunction(functionName, [&]() {
		string values = m_utils.suffixedVariableNameList("value", 0, numVariablesForType(_givenType, _options));
		string encoder = abiEncodingFunction(_givenType, _targetType, _options);
		if (_targetType.isDynamicallyEncoded())
			return Whiskers(R"(
				function <functionName>(<values>, pos) -> updatedPos {
					updatedPos := <encode>(<values>, pos)
				}
			)")
			("functionName", functionName)
			("encode", encoder)
			("values", values)
			.render();
		else
		{
			unsigned encodedSize = _targetType.calldataEncodedSize(_options.padded);
			solAssert(encodedSize != 0, "Invalid encoded size.");
			return Whiskers(R"(
				function <functionName>(<values>, pos) -> updatedPos {
					<encode>(<values>, pos)
					updatedPos := add(pos, <encodedSize>)
				}
			)")
			("functionName", functionName)
			("encode", encoder)
			("encodedSize", toCompactHexWithPrefix(encodedSize))
			("values", values)
			.render();
		}
	});
}

string ABIFunctions::abiEncodingFunctionCalldataArrayWithoutCleanup(
	Type const& _from,
	Type const& _to,
	EncodingOptions const& _options
)
{
	solAssert(_from.category() == Type::Category::Array, "Unknown dynamic type.");
	solAssert(_to.category() == Type::Category::Array, "Unknown dynamic type.");
	auto const& fromArrayType = dynamic_cast<ArrayType const&>(_from);
	auto const& toArrayType = dynamic_cast<ArrayType const&>(_to);

	solAssert(fromArrayType.location() == DataLocation::CallData, "");
	solAssert(
		fromArrayType.isByteArray() ||
		*fromArrayType.baseType() == *TypeProvider::integerType() ||
		*fromArrayType.baseType() == FixedBytesType(32),
	"");
	solAssert(fromArrayType.calldataStride() == toArrayType.memoryStride(), "");

	solAssert(
		*fromArrayType.copyForLocation(DataLocation::Memory, true) ==
		*toArrayType.copyForLocation(DataLocation::Memory, true),
		""
	);

	string functionName =
		"abi_encode_" +
		_from.identifier() +
		"_to_" +
		_to.identifier() +
		_options.toFunctionNameSuffix();
	return createFunction(functionName, [&]() {
		bool needsPadding = _options.padded && fromArrayType.isByteArray();
		if (fromArrayType.isDynamicallySized())
		{
			Whiskers templ(R"(
				// <readableTypeNameFrom> -> <readableTypeNameTo>
				function <functionName>(start, length, pos) -> end {
					pos := <storeLength>(pos, length)
					<scaleLengthByStride>
					<copyFun>(start, pos, length)
					end := add(pos, <lengthPadded>)
				}
			)");
			templ("storeLength", arrayStoreLengthForEncodingFunction(toArrayType, _options));
			templ("functionName", functionName);
			if (fromArrayType.isByteArray() || fromArrayType.calldataStride() == 1)
				templ("scaleLengthByStride", "");
			else
				templ("scaleLengthByStride",
					Whiskers(R"(
						if gt(length, <maxLength>) { revert(0, 0) }
						length := mul(length, <stride>)
					)")
					("stride", toCompactHexWithPrefix(fromArrayType.calldataStride()))
					("maxLength", toCompactHexWithPrefix(u256(-1) / fromArrayType.calldataStride()))
					.render()
				);
			templ("readableTypeNameFrom", _from.toString(true));
			templ("readableTypeNameTo", _to.toString(true));
			templ("copyFun", m_utils.copyToMemoryFunction(true));
			templ("lengthPadded", needsPadding ? m_utils.roundUpFunction() + "(length)" : "length");
			return templ.render();
		}
		else
		{
			solAssert(fromArrayType.calldataStride() == 32, "");
			Whiskers templ(R"(
				// <readableTypeNameFrom> -> <readableTypeNameTo>
				function <functionName>(start, pos) {
					<copyFun>(start, pos, <byteLength>)
				}
			)");
			templ("functionName", functionName);
			templ("readableTypeNameFrom", _from.toString(true));
			templ("readableTypeNameTo", _to.toString(true));
			templ("copyFun", m_utils.copyToMemoryFunction(true));
			templ("byteLength", toCompactHexWithPrefix(fromArrayType.length() * fromArrayType.calldataStride()));
			return templ.render();
		}
	});
}

string ABIFunctions::abiEncodingFunctionSimpleArray(
	ArrayType const& _from,
	ArrayType const& _to,
	EncodingOptions const& _options
)
{
	string functionName =
		"abi_encode_" +
		_from.identifier() +
		"_to_" +
		_to.identifier() +
		_options.toFunctionNameSuffix();

	solAssert(_from.isDynamicallySized() == _to.isDynamicallySized(), "");
	solAssert(_from.length() == _to.length(), "");
	solAssert(!_from.isByteArray(), "");
	if (_from.dataStoredIn(DataLocation::Storage))
		solAssert(_from.baseType()->storageBytes() > 16, "");

	return createFunction(functionName, [&]() {
		bool dynamic = _to.isDynamicallyEncoded();
		bool dynamicBase = _to.baseType()->isDynamicallyEncoded();
		bool const usesTail = dynamicBase && !_options.dynamicInplace;
		EncodingOptions subOptions(_options);
		subOptions.encodeFunctionFromStack = false;
		subOptions.padded = true;
		string elementValues = m_utils.suffixedVariableNameList("elementValue", 0, numVariablesForType(*_from.baseType(), subOptions));
		Whiskers templ(
			usesTail ?
			R"(
				// <readableTypeNameFrom> -> <readableTypeNameTo>
				function <functionName>(value,<maybeLength> pos) <return> {
					<declareLength>
					pos := <storeLength>(pos, length)
					let headStart := pos
					let tail := add(pos, mul(length, 0x20))
					let baseRef := <dataAreaFun>(value)
					let srcPtr := baseRef
					for { let i := 0 } lt(i, length) { i := add(i, 1) }
					{
						mstore(pos, sub(tail, headStart))
						let <elementValues> := <arrayElementAccess>
						tail := <encodeToMemoryFun>(<elementValues>, tail)
						srcPtr := <nextArrayElement>(srcPtr)
						pos := add(pos, 0x20)
					}
					pos := tail
					<assignEnd>
				}
			)" :
			R"(
				// <readableTypeNameFrom> -> <readableTypeNameTo>
				function <functionName>(value,<maybeLength> pos) <return> {
					<declareLength>
					pos := <storeLength>(pos, length)
					let baseRef := <dataAreaFun>(value)
					let srcPtr := baseRef
					for { let i := 0 } lt(i, length) { i := add(i, 1) }
					{
						let <elementValues> := <arrayElementAccess>
						pos := <encodeToMemoryFun>(<elementValues>, pos)
						srcPtr := <nextArrayElement>(srcPtr)
					}
					<assignEnd>
				}
			)"
		);
		templ("functionName", functionName);
		templ("elementValues", elementValues);
		bool lengthAsArgument = _from.dataStoredIn(DataLocation::CallData) && _from.isDynamicallySized();
		if (lengthAsArgument)
		{
			templ("maybeLength", " length,");
			templ("declareLength", "");
		}
		else
		{
			templ("maybeLength", "");
			templ("declareLength", "let length := " + m_utils.arrayLengthFunction(_from) + "(value)");
		}
		templ("readableTypeNameFrom", _from.toString(true));
		templ("readableTypeNameTo", _to.toString(true));
		templ("return", dynamic ? " -> end " : "");
		templ("assignEnd", dynamic ? "end := pos" : "");
		templ("storeLength", arrayStoreLengthForEncodingFunction(_to, _options));
		templ("dataAreaFun", m_utils.arrayDataAreaFunction(_from));

		templ("encodeToMemoryFun", abiEncodeAndReturnUpdatedPosFunction(*_from.baseType(), *_to.baseType(), subOptions));
		switch (_from.location())
		{
			case DataLocation::Memory:
				templ("arrayElementAccess", "mload(srcPtr)");
				break;
			case DataLocation::Storage:
				if (_from.baseType()->isValueType())
					templ("arrayElementAccess", readFromStorage(*_from.baseType(), 0, false) + "(srcPtr)");
				else
					templ("arrayElementAccess", "srcPtr");
				break;
			case DataLocation::CallData:
				templ("arrayElementAccess", calldataAccessFunction(*_from.baseType()) + "(baseRef, srcPtr)");
				break;
			default:
				solAssert(false, "");
		}
		templ("nextArrayElement", m_utils.nextArrayElementFunction(_from));
		return templ.render();
	});
}

string ABIFunctions::abiEncodingFunctionMemoryByteArray(
	ArrayType const& _from,
	ArrayType const& _to,
	EncodingOptions const& _options
)
{
	string functionName =
		"abi_encode_" +
		_from.identifier() +
		"_to_" +
		_to.identifier() +
		_options.toFunctionNameSuffix();

	solAssert(_from.isDynamicallySized() == _to.isDynamicallySized(), "");
	solAssert(_from.length() == _to.length(), "");
	solAssert(_from.dataStoredIn(DataLocation::Memory), "");
	solAssert(_from.isByteArray(), "");

	return createFunction(functionName, [&]() {
		solAssert(_to.isByteArray(), "");
		Whiskers templ(R"(
			function <functionName>(value, pos) -> end {
				let length := <lengthFun>(value)
				pos := <storeLength>(pos, length)
				<copyFun>(add(value, 0x20), pos, length)
				end := add(pos, <lengthPadded>)
			}
		)");
		templ("functionName", functionName);
		templ("lengthFun", m_utils.arrayLengthFunction(_from));
		templ("storeLength", arrayStoreLengthForEncodingFunction(_to, _options));
		templ("copyFun", m_utils.copyToMemoryFunction(false));
		templ("lengthPadded", _options.padded ? m_utils.roundUpFunction() + "(length)" : "length");
		return templ.render();
	});
}

string ABIFunctions::abiEncodingFunctionCompactStorageArray(
	ArrayType const& _from,
	ArrayType const& _to,
	EncodingOptions const& _options
)
{
	string functionName =
		"abi_encode_" +
		_from.identifier() +
		"_to_" +
		_to.identifier() +
		_options.toFunctionNameSuffix();

	solAssert(_from.isDynamicallySized() == _to.isDynamicallySized(), "");
	solAssert(_from.length() == _to.length(), "");
	solAssert(_from.dataStoredIn(DataLocation::Storage), "");

	return createFunction(functionName, [&]() {
		if (_from.isByteArray())
		{
			solAssert(_to.isByteArray(), "");
			Whiskers templ(R"(
				// <readableTypeNameFrom> -> <readableTypeNameTo>
				function <functionName>(value, pos) -> ret {
					let slotValue := sload(value)
					switch and(slotValue, 1)
					case 0 {
						// short byte array
						let length := and(div(slotValue, 2), 0x7f)
						pos := <storeLength>(pos, length)
						mstore(pos, and(slotValue, not(0xff)))
						ret := add(pos, <lengthPaddedShort>)
					}
					case 1 {
						// long byte array
						let length := div(slotValue, 2)
						pos := <storeLength>(pos, length)
						let dataPos := <arrayDataSlot>(value)
						let i := 0
						for { } lt(i, length) { i := add(i, 0x20) } {
							mstore(add(pos, i), sload(dataPos))
							dataPos := add(dataPos, 1)
						}
						ret := add(pos, <lengthPaddedLong>)
					}
				}
			)");
			templ("functionName", functionName);
			templ("readableTypeNameFrom", _from.toString(true));
			templ("readableTypeNameTo", _to.toString(true));
			templ("storeLength", arrayStoreLengthForEncodingFunction(_to, _options));
			templ("lengthPaddedShort", _options.padded ? "0x20" : "length");
			templ("lengthPaddedLong", _options.padded ? "i" : "length");
			templ("arrayDataSlot", m_utils.arrayDataAreaFunction(_from));
			return templ.render();
		}
		else
		{
			// Multiple items per slot
			solAssert(_from.baseType()->storageBytes() <= 16, "");
			solAssert(!_from.baseType()->isDynamicallyEncoded(), "");
			solAssert(_from.baseType()->isValueType(), "");
			bool dynamic = _to.isDynamicallyEncoded();
			size_t storageBytes = _from.baseType()->storageBytes();
			size_t itemsPerSlot = 32 / storageBytes;
			solAssert(itemsPerSlot > 0, "");
			// The number of elements we need to handle manually after the loop.
			size_t spill = size_t(_from.length() % itemsPerSlot);
			Whiskers templ(
				R"(
					// <readableTypeNameFrom> -> <readableTypeNameTo>
					function <functionName>(value, pos) <return> {
						let length := <lengthFun>(value)
						pos := <storeLength>(pos, length)
						let originalPos := pos
						let srcPtr := <dataArea>(value)
						let itemCounter := 0
						if <useLoop> {
							// Run the loop over all full slots
							for { } lt(add(itemCounter, sub(<itemsPerSlot>, 1)), length)
										{ itemCounter := add(itemCounter, <itemsPerSlot>) }
							{
								let data := sload(srcPtr)
								<#items>
									<encodeToMemoryFun>(<extractFromSlot>(data), pos)
									pos := add(pos, <elementEncodedSize>)
								</items>
								srcPtr := add(srcPtr, 1)
							}
						}
						// Handle the last (not necessarily full) slot specially
						if <useSpill> {
							let data := sload(srcPtr)
							<#items>
								if <inRange> {
									<encodeToMemoryFun>(<extractFromSlot>(data), pos)
									pos := add(pos, <elementEncodedSize>)
									itemCounter := add(itemCounter, 1)
								}
							</items>
						}
						<assignEnd>
					}
				)"
			);
			templ("functionName", functionName);
			templ("readableTypeNameFrom", _from.toString(true));
			templ("readableTypeNameTo", _to.toString(true));
			templ("return", dynamic ? " -> end " : "");
			templ("assignEnd", dynamic ? "end := pos" : "");
			templ("lengthFun", m_utils.arrayLengthFunction(_from));
			templ("storeLength", arrayStoreLengthForEncodingFunction(_to, _options));
			templ("dataArea", m_utils.arrayDataAreaFunction(_from));
			// We skip the loop for arrays that fit a single slot.
			if (_from.isDynamicallySized() || _from.length() >= itemsPerSlot)
				templ("useLoop", "1");
			else
				templ("useLoop", "0");
			if (_from.isDynamicallySized() || spill != 0)
				templ("useSpill", "1");
			else
				templ("useSpill", "0");
			templ("itemsPerSlot", to_string(itemsPerSlot));
			// We use padded size because array elements are always padded.
			string elementEncodedSize = toCompactHexWithPrefix(_to.baseType()->calldataEncodedSize());
			templ("elementEncodedSize", elementEncodedSize);

			EncodingOptions subOptions(_options);
			subOptions.encodeFunctionFromStack = false;
			subOptions.padded = true;
			string encodeToMemoryFun = abiEncodingFunction(
				*_from.baseType(),
				*_to.baseType(),
				subOptions
			);
			templ("encodeToMemoryFun", encodeToMemoryFun);
			std::vector<std::map<std::string, std::string>> items(itemsPerSlot);
			for (size_t i = 0; i < itemsPerSlot; ++i)
			{
				if (_from.isDynamicallySized())
					items[i]["inRange"] = "lt(itemCounter, length)";
				else if (i < spill)
					items[i]["inRange"] = "1";
				else
					items[i]["inRange"] = "0";
				items[i]["extractFromSlot"] = extractFromStorageValue(*_from.baseType(), i * storageBytes, false);
			}
			templ("items", items);
			return templ.render();
		}
	});
}

string ABIFunctions::abiEncodingFunctionStruct(
	StructType const& _from,
	StructType const& _to,
	EncodingOptions const& _options
)
{
	string functionName =
		"abi_encode_" +
		_from.identifier() +
		"_to_" +
		_to.identifier() +
		_options.toFunctionNameSuffix();

	solAssert(&_from.structDefinition() == &_to.structDefinition(), "");

	return createFunction(functionName, [&]() {
		bool dynamic = _to.isDynamicallyEncoded();
		Whiskers templ(R"(
			// <readableTypeNameFrom> -> <readableTypeNameTo>
			function <functionName>(value, pos) <return> {
				let tail := add(pos, <headSize>)
				<init>
				<#members>
				{
					// <memberName>
					<preprocess>
					let <memberValues> := <retrieveValue>
					<encode>
				}
				</members>
				<assignEnd>
			}
		)");
		templ("functionName", functionName);
		templ("readableTypeNameFrom", _from.toString(true));
		templ("readableTypeNameTo", _to.toString(true));
		templ("return", dynamic ? " -> end " : "");
		if (dynamic && _options.dynamicInplace)
			templ("assignEnd", "end := pos");
		else if (dynamic && !_options.dynamicInplace)
			templ("assignEnd", "end := tail");
		else
			templ("assignEnd", "");
		// to avoid multiple loads from the same slot for subsequent members
		templ("init", _from.dataStoredIn(DataLocation::Storage) ? "let slotValue := 0" : "");
		u256 previousSlotOffset(-1);
		u256 encodingOffset = 0;
		vector<map<string, string>> members;
		for (auto const& member: _to.members(nullptr))
		{
			solAssert(member.type, "");
			if (!member.type->canLiveOutsideStorage())
				continue;
			TypePointer memberTypeTo = member.type->fullEncodingType(_options.encodeAsLibraryTypes, true, false);
			solUnimplementedAssert(memberTypeTo, "Encoding type \"" + member.type->toString() + "\" not yet implemented.");
			auto memberTypeFrom = _from.memberType(member.name);
			solAssert(memberTypeFrom, "");
			bool dynamicMember = memberTypeTo->isDynamicallyEncoded();
			if (dynamicMember)
				solAssert(dynamic, "");

			members.push_back({});
			members.back()["preprocess"] = "";

			switch (_from.location())
			{
				case DataLocation::Storage:
				{
					solAssert(memberTypeFrom->isValueType() == memberTypeTo->isValueType(), "");
					u256 storageSlotOffset;
					size_t intraSlotOffset;
					tie(storageSlotOffset, intraSlotOffset) = _from.storageOffsetsOfMember(member.name);
					if (memberTypeFrom->isValueType())
					{
						if (storageSlotOffset != previousSlotOffset)
						{
							members.back()["preprocess"] = "slotValue := sload(add(value, " + toCompactHexWithPrefix(storageSlotOffset) + "))";
							previousSlotOffset = storageSlotOffset;
						}
						members.back()["retrieveValue"] = extractFromStorageValue(*memberTypeFrom, intraSlotOffset, false) + "(slotValue)";
					}
					else
					{
						solAssert(memberTypeFrom->dataStoredIn(DataLocation::Storage), "");
						solAssert(intraSlotOffset == 0, "");
						members.back()["retrieveValue"] = "add(value, " + toCompactHexWithPrefix(storageSlotOffset) + ")";
					}
					break;
				}
				case DataLocation::Memory:
				{
					string sourceOffset = toCompactHexWithPrefix(_from.memoryOffsetOfMember(member.name));
					members.back()["retrieveValue"] = "mload(add(value, " + sourceOffset + "))";
					break;
				}
				case DataLocation::CallData:
				{
					string sourceOffset = toCompactHexWithPrefix(_from.calldataOffsetOfMember(member.name));
					members.back()["retrieveValue"] = calldataAccessFunction(*memberTypeFrom) + "(value, add(value, " + sourceOffset + "))";
					break;
				}
				default:
					solAssert(false, "");
			}

			EncodingOptions subOptions(_options);
			subOptions.encodeFunctionFromStack = false;
			// Like with arrays, struct members are always padded.
			subOptions.padded = true;

			string memberValues = m_utils.suffixedVariableNameList("memberValue", 0, numVariablesForType(*memberTypeFrom, subOptions));
			members.back()["memberValues"] = memberValues;

			string encode;
			if (_options.dynamicInplace)
				encode = Whiskers{"pos := <encode>(<memberValues>, pos)"}
					("encode", abiEncodeAndReturnUpdatedPosFunction(*memberTypeFrom, *memberTypeTo, subOptions))
					("memberValues", memberValues)
					.render();
			else
			{
				Whiskers encodeTempl(
					dynamicMember ?
					string(R"(
						mstore(add(pos, <encodingOffset>), sub(tail, pos))
						tail := <abiEncode>(<memberValues>, tail)
					)") :
					"<abiEncode>(<memberValues>, add(pos, <encodingOffset>))"
				);
				encodeTempl("memberValues", memberValues);
				encodeTempl("encodingOffset", toCompactHexWithPrefix(encodingOffset));
				encodingOffset += dynamicMember ? 0x20 : memberTypeTo->calldataEncodedSize();
				encodeTempl("abiEncode", abiEncodingFunction(*memberTypeFrom, *memberTypeTo, subOptions));
				encode = encodeTempl.render();
			}
			members.back()["encode"] = encode;

			members.back()["memberName"] = member.name;
		}
		templ("members", members);
		if (_options.dynamicInplace)
			solAssert(encodingOffset == 0, "In-place encoding should enforce zero head size.");
		templ("headSize", toCompactHexWithPrefix(encodingOffset));
		return templ.render();
	});
}

string ABIFunctions::abiEncodingFunctionStringLiteral(
	Type const& _from,
	Type const& _to,
	EncodingOptions const& _options
)
{
	solAssert(_from.category() == Type::Category::StringLiteral, "");

	string functionName =
		"abi_encode_" +
		_from.identifier() +
		"_to_" +
		_to.identifier() +
		_options.toFunctionNameSuffix();
	return createFunction(functionName, [&]() {
		auto const& strType = dynamic_cast<StringLiteralType const&>(_from);
		string const& value = strType.value();
		solAssert(_from.sizeOnStack() == 0, "");

		if (_to.isDynamicallySized())
		{
			solAssert(_to.category() == Type::Category::Array, "");
			Whiskers templ(R"(
				function <functionName>(pos) -> end {
					pos := <storeLength>(pos, <length>)
					<#word>
						mstore(add(pos, <offset>), <wordValue>)
					</word>
					end := add(pos, <overallSize>)
				}
			)");
			templ("functionName", functionName);

			// TODO this can make use of CODECOPY for large strings once we have that in Yul
			size_t words = (value.size() + 31) / 32;
			templ("length", to_string(value.size()));
			templ("storeLength", arrayStoreLengthForEncodingFunction(dynamic_cast<ArrayType const&>(_to), _options));
			if (_options.padded)
				templ("overallSize", to_string(words * 32));
			else
				templ("overallSize", to_string(value.size()));

			vector<map<string, string>> wordParams(words);
			for (size_t i = 0; i < words; ++i)
			{
				wordParams[i]["offset"] = to_string(i * 32);
				wordParams[i]["wordValue"] = "0x" + h256(value.substr(32 * i, 32), h256::AlignLeft).hex();
			}
			templ("word", wordParams);
			return templ.render();
		}
		else
		{
			solAssert(_to.category() == Type::Category::FixedBytes, "");
			solAssert(value.size() <= 32, "");
			Whiskers templ(R"(
				function <functionName>(pos) {
					mstore(pos, <wordValue>)
				}
			)");
			templ("functionName", functionName);
			templ("wordValue", "0x" + h256(value, h256::AlignLeft).hex());
			return templ.render();
		}
	});
}

string ABIFunctions::abiEncodingFunctionFunctionType(
	FunctionType const& _from,
	Type const& _to,
	EncodingOptions const& _options
)
{
	solAssert(_from.kind() == FunctionType::Kind::External, "");
	solAssert(_from == _to, "");

	string functionName =
		"abi_encode_" +
		_from.identifier() +
		"_to_" +
		_to.identifier() +
		_options.toFunctionNameSuffix();

	if (_options.encodeFunctionFromStack)
		return createFunction(functionName, [&]() {
			return Whiskers(R"(
				function <functionName>(addr, function_id, pos) {
					mstore(pos, <combineExtFun>(addr, function_id))
				}
			)")
			("functionName", functionName)
			("combineExtFun", m_utils.combineExternalFunctionIdFunction())
			.render();
		});
	else
		return createFunction(functionName, [&]() {
			return Whiskers(R"(
				function <functionName>(addr_and_function_id, pos) {
					mstore(pos, <cleanExtFun>(addr_and_function_id))
				}
			)")
			("functionName", functionName)
			("cleanExtFun", m_utils.cleanupFunction(_to))
			.render();
		});
}

string ABIFunctions::abiDecodingFunction(Type const& _type, bool _fromMemory, bool _forUseOnStack)
{
	// The decoding function has to perform bounds checks unless it decodes a value type.
	// Conversely, bounds checks have to be performed before the decoding function
	// of a value type is called.

	TypePointer decodingType = _type.decodingType();
	solAssert(decodingType, "");

	if (auto arrayType = dynamic_cast<ArrayType const*>(decodingType))
	{
		if (arrayType->dataStoredIn(DataLocation::CallData))
		{
			solAssert(!_fromMemory, "");
			return abiDecodingFunctionCalldataArray(*arrayType);
		}
		else if (arrayType->isByteArray())
			return abiDecodingFunctionByteArray(*arrayType, _fromMemory);
		else
			return abiDecodingFunctionArray(*arrayType, _fromMemory);
	}
	else if (auto const* structType = dynamic_cast<StructType const*>(decodingType))
	{
		if (structType->dataStoredIn(DataLocation::CallData))
		{
			solAssert(!_fromMemory, "");
			return abiDecodingFunctionCalldataStruct(*structType);
		}
		else
			return abiDecodingFunctionStruct(*structType, _fromMemory);
	}
	else if (auto const* functionType = dynamic_cast<FunctionType const*>(decodingType))
		return abiDecodingFunctionFunctionType(*functionType, _fromMemory, _forUseOnStack);
	else
		return abiDecodingFunctionValueType(_type, _fromMemory);
}

string ABIFunctions::abiDecodingFunctionValueType(Type const& _type, bool _fromMemory)
{
	TypePointer decodingType = _type.decodingType();
	solAssert(decodingType, "");
	solAssert(decodingType->sizeOnStack() == 1, "");
	solAssert(decodingType->isValueType(), "");
	solAssert(decodingType->calldataEncodedSize() == 32, "");
	solAssert(!decodingType->isDynamicallyEncoded(), "");

	string functionName =
		"abi_decode_" +
		_type.identifier() +
		(_fromMemory ? "_fromMemory" : "");
	return createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(offset, end) -> value {
				value := <load>(offset)
				<validator>(value)
			}
		)");
		templ("functionName", functionName);
		templ("load", _fromMemory ? "mload" : "calldataload");
		// Validation should use the type and not decodingType, because e.g.
		// the decoding type of an enum is a plain int.
		templ("validator", m_utils.validatorFunction(_type, true));
		return templ.render();
	});

}

string ABIFunctions::abiDecodingFunctionArray(ArrayType const& _type, bool _fromMemory)
{
	solAssert(_type.dataStoredIn(DataLocation::Memory), "");
	solAssert(!_type.isByteArray(), "");

	string functionName =
		"abi_decode_" +
		_type.identifier() +
		(_fromMemory ? "_fromMemory" : "");

	solAssert(!_type.dataStoredIn(DataLocation::Storage), "");

	return createFunction(functionName, [&]() {
		string load = _fromMemory ? "mload" : "calldataload";
		bool dynamicBase = _type.baseType()->isDynamicallyEncoded();
		Whiskers templ(
			R"(
				// <readableTypeName>
				function <functionName>(offset, end) -> array {
					if iszero(slt(add(offset, 0x1f), end)) { revert(0, 0) }
					let length := <retrieveLength>
					array := <allocate>(<allocationSize>(length))
					let dst := array
					<storeLength> // might update offset and dst
					let src := offset
					<staticBoundsCheck>
					for { let i := 0 } lt(i, length) { i := add(i, 1) }
					{
						let elementPos := <retrieveElementPos>
						mstore(dst, <decodingFun>(elementPos, end))
						dst := add(dst, 0x20)
						src := add(src, <baseEncodedSize>)
					}
				}
			)"
		);
		templ("functionName", functionName);
		templ("readableTypeName", _type.toString(true));
		templ("retrieveLength", !_type.isDynamicallySized() ? toCompactHexWithPrefix(_type.length()) : load + "(offset)");
		templ("allocate", m_utils.allocationFunction());
		templ("allocationSize", m_utils.arrayAllocationSizeFunction(_type));
		if (_type.isDynamicallySized())
			templ("storeLength", "mstore(array, length) offset := add(offset, 0x20) dst := add(dst, 0x20)");
		else
			templ("storeLength", "");
		if (dynamicBase)
		{
			templ("staticBoundsCheck", "");
			templ("retrieveElementPos", "add(offset, " + load + "(src))");
			templ("baseEncodedSize", "0x20");
		}
		else
		{
			string baseEncodedSize = toCompactHexWithPrefix(_type.baseType()->calldataEncodedSize());
			templ("staticBoundsCheck", "if gt(add(src, mul(length, " + baseEncodedSize + ")), end) { revert(0, 0) }");
			templ("retrieveElementPos", "src");
			templ("baseEncodedSize", baseEncodedSize);
		}
		templ("decodingFun", abiDecodingFunction(*_type.baseType(), _fromMemory, false));
		return templ.render();
	});
}

string ABIFunctions::abiDecodingFunctionCalldataArray(ArrayType const& _type)
{
	solAssert(_type.dataStoredIn(DataLocation::CallData), "");
	if (!_type.isDynamicallySized())
		solAssert(_type.length() < u256("0xffffffffffffffff"), "");
	solAssert(_type.baseType()->calldataEncodedSize() > 0, "");
	solAssert(_type.baseType()->calldataEncodedSize() < u256("0xffffffffffffffff"), "");

	string functionName =
		"abi_decode_" +
		_type.identifier();
	return createFunction(functionName, [&]() {
		string templ;
		if (_type.isDynamicallySized())
			templ = R"(
				// <readableTypeName>
				function <functionName>(offset, end) -> arrayPos, length {
					if iszero(slt(add(offset, 0x1f), end)) { revert(0, 0) }
					length := calldataload(offset)
					if gt(length, 0xffffffffffffffff) { revert(0, 0) }
					arrayPos := add(offset, 0x20)
					if gt(add(arrayPos, mul(length, <baseEncodedSize>)), end) { revert(0, 0) }
				}
			)";
		else
			templ = R"(
				// <readableTypeName>
				function <functionName>(offset, end) -> arrayPos {
					arrayPos := offset
					if gt(add(arrayPos, mul(<length>, <baseEncodedSize>)), end) { revert(0, 0) }
				}
			)";
		Whiskers w{templ};
		w("functionName", functionName);
		w("readableTypeName", _type.toString(true));
		w("baseEncodedSize", toCompactHexWithPrefix(_type.isByteArray() ? 1 : _type.baseType()->calldataEncodedSize()));
		if (!_type.isDynamicallySized())
			w("length", toCompactHexWithPrefix(_type.length()));
		return w.render();
	});
}

string ABIFunctions::abiDecodingFunctionByteArray(ArrayType const& _type, bool _fromMemory)
{
	solAssert(_type.dataStoredIn(DataLocation::Memory), "");
	solAssert(_type.isByteArray(), "");

	string functionName =
		"abi_decode_" +
		_type.identifier() +
		(_fromMemory ? "_fromMemory" : "");

	return createFunction(functionName, [&]() {
		Whiskers templ(
			R"(
				function <functionName>(offset, end) -> array {
					if iszero(slt(add(offset, 0x1f), end)) { revert(0, 0) }
					let length := <load>(offset)
					array := <allocate>(<allocationSize>(length))
					mstore(array, length)
					let src := add(offset, 0x20)
					let dst := add(array, 0x20)
					if gt(add(src, length), end) { revert(0, 0) }
					<copyToMemFun>(src, dst, length)
				}
			)"
		);
		templ("functionName", functionName);
		templ("load", _fromMemory ? "mload" : "calldataload");
		templ("allocate", m_utils.allocationFunction());
		templ("allocationSize", m_utils.arrayAllocationSizeFunction(_type));
		templ("copyToMemFun", m_utils.copyToMemoryFunction(!_fromMemory));
		return templ.render();
	});
}

string ABIFunctions::abiDecodingFunctionCalldataStruct(StructType const& _type)
{
	solAssert(_type.dataStoredIn(DataLocation::CallData), "");
	solAssert(_type.calldataEncodedSize(true) != 0, "");
	string functionName =
		"abi_decode_" +
		_type.identifier();

	return createFunction(functionName, [&]() {
		Whiskers w{R"(
				// <readableTypeName>
				function <functionName>(offset, end) -> value {
					if slt(sub(end, offset), <minimumSize>) { revert(0, 0) }
					value := offset
				}
		)"};
		w("functionName", functionName);
		w("readableTypeName", _type.toString(true));
		w("minimumSize", to_string(_type.calldataEncodedSize(true)));
		return w.render();
	});
}

string ABIFunctions::abiDecodingFunctionStruct(StructType const& _type, bool _fromMemory)
{
	solAssert(!_type.dataStoredIn(DataLocation::CallData), "");
	string functionName =
		"abi_decode_" +
		_type.identifier() +
		(_fromMemory ? "_fromMemory" : "");

	return createFunction(functionName, [&]() {
		Whiskers templ(R"(
			// <readableTypeName>
			function <functionName>(headStart, end) -> value {
				if slt(sub(end, headStart), <minimumSize>) { revert(0, 0) }
				value := <allocate>(<memorySize>)
				<#members>
				{
					// <memberName>
					<decode>
				}
				</members>
			}
		)");
		templ("functionName", functionName);
		templ("readableTypeName", _type.toString(true));
		templ("allocate", m_utils.allocationFunction());
		solAssert(_type.memorySize() < u256("0xffffffffffffffff"), "");
		templ("memorySize", toCompactHexWithPrefix(_type.memorySize()));
		size_t headPos = 0;
		vector<map<string, string>> members;
		for (auto const& member: _type.members(nullptr))
		{
			solAssert(member.type, "");
			solAssert(member.type->canLiveOutsideStorage(), "");
			auto decodingType = member.type->decodingType();
			solAssert(decodingType, "");
			bool dynamic = decodingType->isDynamicallyEncoded();
			Whiskers memberTempl(
				dynamic ?
				R"(
					let offset := <load>(add(headStart, <pos>))
					if gt(offset, 0xffffffffffffffff) { revert(0, 0) }
					mstore(add(value, <memoryOffset>), <abiDecode>(add(headStart, offset), end))
				)" :
				R"(
					let offset := <pos>
					mstore(add(value, <memoryOffset>), <abiDecode>(add(headStart, offset), end))
				)"
			);
			memberTempl("load", _fromMemory ? "mload" : "calldataload");
			memberTempl("pos", to_string(headPos));
			memberTempl("memoryOffset", toCompactHexWithPrefix(_type.memoryOffsetOfMember(member.name)));
			memberTempl("abiDecode", abiDecodingFunction(*member.type, _fromMemory, false));

			members.push_back({});
			members.back()["decode"] = memberTempl.render();
			members.back()["memberName"] = member.name;
			headPos += dynamic ? 0x20 : decodingType->calldataEncodedSize();
		}
		templ("members", members);
		templ("minimumSize", toCompactHexWithPrefix(headPos));
		return templ.render();
	});
}

string ABIFunctions::abiDecodingFunctionFunctionType(FunctionType const& _type, bool _fromMemory, bool _forUseOnStack)
{
	solAssert(_type.kind() == FunctionType::Kind::External, "");

	string functionName =
		"abi_decode_" +
		_type.identifier() +
		(_fromMemory ? "_fromMemory" : "") +
		(_forUseOnStack ? "_onStack" : "");

	return createFunction(functionName, [&]() {
		if (_forUseOnStack)
		{
			return Whiskers(R"(
				function <functionName>(offset, end) -> addr, function_selector {
					addr, function_selector := <splitExtFun>(<decodeFun>(offset, end))
				}
			)")
			("functionName", functionName)
			("decodeFun", abiDecodingFunctionFunctionType(_type, _fromMemory, false))
			("splitExtFun", m_utils.splitExternalFunctionIdFunction())
			.render();
		}
		else
		{
			return Whiskers(R"(
				function <functionName>(offset, end) -> fun {
					fun := <load>(offset)
					<validateExtFun>(fun)
				}
			)")
			("functionName", functionName)
			("load", _fromMemory ? "mload" : "calldataload")
			("validateExtFun", m_utils.validatorFunction(_type, true))
			.render();
		}
	});
}

string ABIFunctions::readFromStorage(Type const& _type, size_t _offset, bool _splitFunctionTypes)
{
	solUnimplementedAssert(!_splitFunctionTypes, "");
	string functionName =
		"read_from_storage_" +
		string(_splitFunctionTypes ? "split_" : "") +
		"offset_" +
		to_string(_offset) +
		_type.identifier();
	return m_functionCollector->createFunction(functionName, [&] {
		solAssert(_type.sizeOnStack() == 1, "");
		return Whiskers(R"(
			function <functionName>(slot) -> value {
				value := <extract>(sload(slot))
			}
		)")
		("functionName", functionName)
		("extract", extractFromStorageValue(_type, _offset, false))
		.render();
	});
}

string ABIFunctions::extractFromStorageValue(Type const& _type, size_t _offset, bool _splitFunctionTypes)
{
	solUnimplementedAssert(!_splitFunctionTypes, "");

	string functionName =
		"extract_from_storage_value_" +
		string(_splitFunctionTypes ? "split_" : "") +
		"offset_" +
		to_string(_offset) +
		_type.identifier();
	return m_functionCollector->createFunction(functionName, [&] {
		return Whiskers(R"(
			function <functionName>(slot_value) -> value {
				value := <cleanupStorage>(<shr>(slot_value))
			}
		)")
		("functionName", functionName)
		("shr", m_utils.shiftRightFunction(_offset * 8))
		("cleanupStorage", cleanupFromStorageFunction(_type, false))
		.render();
	});
}

string ABIFunctions::calldataAccessFunction(Type const& _type)
{
	solAssert(_type.isValueType() || _type.dataStoredIn(DataLocation::CallData), "");
	string functionName = "calldata_access_" + _type.identifier();
	return createFunction(functionName, [&]() {
		if (_type.isDynamicallyEncoded())
		{
			unsigned int baseEncodedSize = _type.calldataEncodedSize();
			solAssert(baseEncodedSize > 1, "");
			Whiskers w(R"(
				function <functionName>(base_ref, ptr) -> <return> {
					let rel_offset_of_tail := calldataload(ptr)
					if iszero(slt(rel_offset_of_tail, sub(sub(calldatasize(), base_ref), sub(<neededLength>, 1)))) { revert(0, 0) }
					value := add(rel_offset_of_tail, base_ref)
					<handleLength>
				}
			)");
			if (_type.isDynamicallySized())
			{
				auto const* arrayType = dynamic_cast<ArrayType const*>(&_type);
				solAssert(!!arrayType, "");
				unsigned int calldataStride = arrayType->calldataStride();
				w("handleLength", Whiskers(R"(
					length := calldataload(value)
					value := add(value, 0x20)
					if gt(length, 0xffffffffffffffff) { revert(0, 0) }
					if sgt(base_ref, sub(calldatasize(), mul(length, <calldataStride>))) { revert(0, 0) }
				)")("calldataStride", toCompactHexWithPrefix(calldataStride)).render());
				w("return", "value, length");
			}
			else
			{
				w("handleLength", "");
				w("return", "value");
			}
			w("neededLength", toCompactHexWithPrefix(baseEncodedSize));
			w("functionName", functionName);
			return w.render();
		}
		else if (_type.isValueType())
		{
			string decodingFunction;
			if (auto const* functionType = dynamic_cast<FunctionType const*>(&_type))
				decodingFunction = abiDecodingFunctionFunctionType(*functionType, false, false);
			else
				decodingFunction = abiDecodingFunctionValueType(_type, false);
			// Note that the second argument to the decoding function should be discarded after inlining.
			return Whiskers(R"(
				function <functionName>(baseRef, ptr) -> value {
					value := <decodingFunction>(ptr, add(ptr, 32))
				}
			)")
			("functionName", functionName)
			("decodingFunction", decodingFunction)
			.render();
		}
		else
		{
			solAssert(
				_type.category() == Type::Category::Array ||
				_type.category() == Type::Category::Struct,
				""
			);
			return Whiskers(R"(
				function <functionName>(baseRef, ptr) -> value {
					value := ptr
				}
			)")
			("functionName", functionName)
			.render();
		}
	});
}

string ABIFunctions::arrayStoreLengthForEncodingFunction(ArrayType const& _type, EncodingOptions const& _options)
{
	string functionName = "array_storeLengthForEncoding_" + _type.identifier() + _options.toFunctionNameSuffix();
	return createFunction(functionName, [&]() {
		if (_type.isDynamicallySized() && !_options.dynamicInplace)
			return Whiskers(R"(
				function <functionName>(pos, length) -> updated_pos {
					mstore(pos, length)
					updated_pos := add(pos, 0x20)
				}
			)")
			("functionName", functionName)
			.render();
		else
			return Whiskers(R"(
				function <functionName>(pos, length) -> updated_pos {
					updated_pos := pos
				}
			)")
			("functionName", functionName)
			.render();
	});
}

string ABIFunctions::createFunction(string const& _name, function<string ()> const& _creator)
{
	return m_functionCollector->createFunction(_name, _creator);
}

string ABIFunctions::createExternallyUsedFunction(string const& _name, function<string ()> const& _creator)
{
	string name = createFunction(_name, _creator);
	m_externallyUsedFunctions.insert(name);
	return name;
}

size_t ABIFunctions::headSize(TypePointers const& _targetTypes)
{
	size_t headSize = 0;
	for (auto const& t: _targetTypes)
	{
		if (t->isDynamicallyEncoded())
			headSize += 0x20;
		else
			headSize += t->calldataEncodedSize();
	}

	return headSize;
}

size_t ABIFunctions::numVariablesForType(Type const& _type, EncodingOptions const& _options)
{
	if (_type.category() == Type::Category::Function && !_options.encodeFunctionFromStack)
		return 1;
	else
		return _type.sizeOnStack();
}
