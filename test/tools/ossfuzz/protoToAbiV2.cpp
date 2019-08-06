#include <regex>
#include <numeric>
#include <boost/range/adaptor/reversed.hpp>
#include <test/tools/ossfuzz/protoToAbiV2.h>
#include <libdevcore/StringUtils.h>
#include <libdevcore/Whiskers.h>
#include <liblangutil/Exceptions.h>

using namespace std;
using namespace dev;
using namespace dev::test::abiv2fuzzer;

// Create a new variable declaration and append said variable to function parameter lists
// of coder functions.
// Declared name is x_<i>; parameterized name is c_<i>
// where <i> is a monotonically increasing integer.
void ProtoConverter::createDeclAndParamList(
	std::string const& _type,
	DataType _dataType,
	std::string& _varName,
	std::string& _paramName
)
{
	auto varNames = newVarNames(getNextVarCounter());
	_varName = varNames.first;
	_paramName = varNames.second;

	// Declare array
	appendVarDeclToOutput(_type, _varName, getQualifier(_dataType));

	// Add typed params for calling public and external functions with said type
	appendTypedParams(
		CalleeType::PUBLIC,
		isValueType(_dataType),
		_type,
		_paramName,
		((m_varCounter == 1) ? Delimiter::SKIP : Delimiter::ADD)
	);
	appendTypedParams(
		CalleeType::EXTERNAL,
		isValueType(_dataType),
		_type,
		_paramName,
		((m_varCounter == 1) ? Delimiter::SKIP : Delimiter::ADD)
	);
}

void ProtoConverter::visitArrayType(std::string const& _baseType, ArrayType const& _x)
{
	std::string type = arrayTypeAsString(_baseType, _x);
	std::string varName, paramName;
	createDeclAndParamList(type, DataType::ARRAY, varName, paramName);
	// Resize-initialize array and add checks
	resizeInitArray(_x, _baseType, varName, paramName);
}

void ProtoConverter::visitType(
	DataType _dataType,
	std::string const& _type,
	std::string const& _value
)
{
	std::string varName, paramName;
	createDeclAndParamList(_type, _dataType, varName, paramName);
	addCheckedVarDef(_dataType, varName, paramName, _value);
	// Update right padding of type
	m_isLastParamRightPadded = isDataTypeBytesOrString(_dataType);
}

void ProtoConverter::appendVarDeclToOutput(
	std::string const& _type,
	std::string const& _varName,
	std::string const& _qualifier
)
{
	// One level of indentation for state variable declarations
	// Two levels of indentation for local variable declarations
	m_output << Whiskers(R"(
	<?isLocalVar>	</isLocalVar><type><?qual> <qualifier></qual> <varName>;)"
	)
		("isLocalVar", !m_isStateVar)
		("type", _type)
		("qual", !_qualifier.empty())
		("qualifier", _qualifier)
		("varName", _varName)
		.render();
}

void ProtoConverter::appendChecks(
	DataType _type,
	std::string const& _varName,
	std::string const& _rhs
)
{
	std::string check = {};
	switch (_type)
	{
	case DataType::STRING:
		check = Whiskers(R"(!bytesCompare(bytes(<varName>), <value>))")
			("varName", _varName)
			("value", _rhs)
			.render();
		break;
	case DataType::BYTES:
		check = Whiskers(R"(!bytesCompare(<varName>, <value>))")
			("varName", _varName)
			("value", _rhs)
			.render();
		break;
	case DataType::VALUE:
		check = Whiskers(R"(<varName> != <value>)")
			("varName", _varName)
			("value", _rhs)
			.render();
		break;
	case DataType::ARRAY:
		solUnimplemented("Proto ABIv2 fuzzer: Invalid data type.");
	}

	// Each (failing) check returns a unique value to simplify debugging.
	m_checks << Whiskers(R"(
		if (<check>) return <returnVal>;)"
	)
		("check", check)
		("returnVal", std::to_string(m_returnValue++))
		.render();
}

void ProtoConverter::addVarDef(std::string const& _varName, std::string const& _rhs)
{
	std::string varDefString = Whiskers(R"(
		<varName> = <rhs>;)"
		)
		("varName", _varName)
		("rhs", _rhs)
		.render();

	// State variables cannot be assigned in contract-scope
	// Therefore, we buffer their assignments and
	// render them in function scope later.
	if (m_isStateVar)
		m_statebuffer << varDefString;
	else
		m_output << varDefString;
}

void ProtoConverter::addCheckedVarDef(
	DataType _type,
	std::string const& _varName,
	std::string const& _paramName,
	std::string const& _rhs)
{
	addVarDef(_varName, _rhs);
	appendChecks(_type, _paramName, _rhs);
}

// Runtime check for array length.
void ProtoConverter::checkResizeOp(std::string const& _paramName, unsigned _len)
{
	appendChecks(DataType::VALUE, _paramName + ".length", std::to_string(_len));
}

std::string ProtoConverter::boolValueAsString(unsigned _counter)
{
	return ((_counter % 2) ? "true" : "false");
}

/* Input(s)
 *   - Unsigned integer to be hashed
 *   - Width of desired uint value
 * Processing
 *   - Take hash of first parameter and mask it with the max unsigned value for given bit width
 * Output
 *   - string representation of uint value
 */
std::string ProtoConverter::uintValueAsString(unsigned _width, unsigned _counter)
{
	solAssert(
		(_width % 8 == 0),
		"Proto ABIv2 Fuzzer: Unsigned integer width is not a multiple of 8"
	);
	return maskUnsignedIntToHex(_counter, _width/4);
}

/* Input(s)
 *   - counter to be hashed to derive a value for Integer type
 *   - Width of desired int value
 * Processing
 *   - Take hash of first parameter and mask it with the max signed value for given bit width
 * Output
 *   - string representation of int value
 */
std::string ProtoConverter::intValueAsString(unsigned _width, unsigned _counter)
{
	solAssert(
		(_width % 8 == 0),
		"Proto ABIv2 Fuzzer: Signed integer width is not a multiple of 8"
	);
	return maskUnsignedIntToHex(_counter, ((_width/4) - 1));
}

std::string ProtoConverter::addressValueAsString(unsigned _counter)
{
	return Whiskers(R"(address(<value>))")
		("value", uintValueAsString(160, _counter))
		.render();
}

std::string ProtoConverter::croppedString(
	unsigned _numBytes,
	unsigned _counter,
	bool _isHexLiteral
)
{
	// _numBytes can not be zero or exceed 32 bytes
	solAssert(
		_numBytes > 0 && _numBytes <= 32,
		"Proto ABIv2 fuzzer: Too short or too long a cropped string"
	);

	// Number of masked nibbles is twice the number of bytes for a
	// hex literal of _numBytes bytes. For a string literal, each nibble
	// is treated as a character.
	unsigned numMaskNibbles = _isHexLiteral ? _numBytes * 2 : _numBytes;

	// Start position of substring equals totalHexStringLength - numMaskNibbles
	// totalHexStringLength = 64 + 2 = 66
	// e.g., 0x12345678901234567890123456789012 is a total of 66 characters
	//      |---------------------^-----------|
	//      <--- start position---><--numMask->
	//      <-----------total length --------->
	// Note: This assumes that maskUnsignedIntToHex() invokes toHex(..., HexPrefix::Add)
	unsigned startPos = 66 - numMaskNibbles;
	// Extracts the least significant numMaskNibbles from the result
	// of maskUnsignedIntToHex().
	return maskUnsignedIntToHex(
		_counter,
		numMaskNibbles
	).substr(startPos, numMaskNibbles);
}

std::string ProtoConverter::hexValueAsString(
	unsigned _numBytes,
	unsigned _counter,
	bool _isHexLiteral,
	bool _decorate
)
{
	solAssert(_numBytes > 0 && _numBytes <= 32,
		"Proto ABIv2 fuzzer: Invalid hex length"
	);

	// If _decorate is set, then we return a hex"" or a "" string.
	if (_numBytes == 0)
		return Whiskers(R"(<?decorate><?isHex>hex</isHex>""</decorate>)")
			("decorate", _decorate)
			("isHex", _isHexLiteral)
			.render();

	// This is needed because solidity interprets a 20-byte 0x prefixed hex literal as an address
	// payable type.
	return Whiskers(R"(<?decorate><?isHex>hex</isHex>"</decorate><value><?decorate>"</decorate>)")
		("decorate", _decorate)
		("isHex", _isHexLiteral)
		("value", croppedString(_numBytes, _counter, _isHexLiteral))
		.render();
}

std::string ProtoConverter::variableLengthValueAsString(
	unsigned _numBytes,
	unsigned _counter,
	bool _isHexLiteral
)
{
	solAssert(_numBytes >= 0 && _numBytes <= s_maxDynArrayLength,
		"Proto ABIv2 fuzzer: Invalid hex length"
	);
	if (_numBytes == 0)
		return Whiskers(R"(<?isHex>hex</isHex>"")")
			("isHex", _isHexLiteral)
			.render();

	unsigned numBytesRemaining = _numBytes;
	// Stores the literal
	string output{};
	// If requested value is shorter than or exactly 32 bytes,
	// the literal is the return value of hexValueAsString.
	if (numBytesRemaining <= 32)
		output = hexValueAsString(
			numBytesRemaining,
			_counter,
			_isHexLiteral,
			/*decorate=*/false
		);
	// If requested value is longer than 32 bytes, the literal
	// is obtained by duplicating the return value of hexValueAsString
	// until we reach a value of the requested size.
	else
	{
		// Create a 32-byte value to be duplicated and
		// update number of bytes to be appended.
		// Stores the cached literal that saves us
		// (expensive) calls to keccak256.
		string cachedString = hexValueAsString(
			/*numBytes=*/32,
			_counter,
			_isHexLiteral,
			/*decorate=*/false
		);
		output = cachedString;
		numBytesRemaining -= 32;

		// Append bytes from cachedString until
		// we create a value of desired length.
		unsigned numAppendedBytes;
		while (numBytesRemaining > 0)
		{
			// We append at most 32 bytes at a time
			numAppendedBytes = numBytesRemaining >= 32 ? 32 : numBytesRemaining;
			output += cachedString.substr(
				0,
				// Double the substring length for hex literals since each
				// character is actually half a byte (or a nibble).
				_isHexLiteral ? numAppendedBytes * 2 : numAppendedBytes
			);
			numBytesRemaining -= numAppendedBytes;
		}
		solAssert(
			numBytesRemaining == 0,
			"Proto ABIv2 fuzzer: Logic flaw in variable literal creation"
		);
	}

	if (_isHexLiteral)
		solAssert(
			output.size() == 2 * _numBytes,
			"Proto ABIv2 fuzzer: Generated hex literal is of incorrect length"
		);
	else
		solAssert(
			output.size() == _numBytes,
			"Proto ABIv2 fuzzer: Generated string literal is of incorrect length"
		);

	// Decorate output
	return Whiskers(R"(<?isHexLiteral>hex</isHexLiteral>"<value>")")
		("isHexLiteral", _isHexLiteral)
		("value", output)
		.render();
}

std::string ProtoConverter::fixedByteValueAsString(unsigned _width, unsigned _counter)
{
	solAssert(
		(_width >= 1 && _width <= 32),
		"Proto ABIv2 Fuzzer: Fixed byte width is not between 1--32"
	);
	return hexValueAsString(_width, _counter, /*isHexLiteral=*/true);
}

std::string ProtoConverter::integerValueAsString(bool _sign, unsigned _width, unsigned _counter)
{
	if (_sign)
		return intValueAsString(_width, _counter);
	else
		return uintValueAsString(_width, _counter);
}

std::string ProtoConverter::bytesArrayTypeAsString(DynamicByteArrayType const& _x)
{
	switch (_x.type())
	{
	case DynamicByteArrayType::BYTES:
		return "bytes";
	case DynamicByteArrayType::STRING:
		return "string";
	}
}

std::string ProtoConverter::structTypeAsString(StructType const&)
{
	// TODO: Implement this
	return {};
}

void ProtoConverter::visit(BoolType const&)
{
	visitType(
		DataType::VALUE,
		getBoolTypeAsString(),
		boolValueAsString(getNextCounter())
	);
}

void ProtoConverter::visit(IntegerType const& _x)
{
	visitType(
		DataType::VALUE,
		getIntTypeAsString(_x),
		integerValueAsString(isIntSigned(_x), getIntWidth(_x), getNextCounter())
	);
}

void ProtoConverter::visit(AddressType const& _x)
{
	visitType(
		DataType::VALUE,
		getAddressTypeAsString(_x),
		addressValueAsString(getNextCounter())
	);
}

void ProtoConverter::visit(FixedByteType const& _x)
{
	visitType(
		DataType::VALUE,
		getFixedByteTypeAsString(_x),
		fixedByteValueAsString(getFixedByteWidth(_x), getNextCounter())
	);
}

void ProtoConverter::visit(ValueType const& _x)
{
	switch (_x.value_type_oneof_case())
	{
		case ValueType::kInty:
			visit(_x.inty());
			break;
		case ValueType::kByty:
			visit(_x.byty());
			break;
		case ValueType::kAdty:
			visit(_x.adty());
			break;
		case ValueType::kBoolty:
			visit(_x.boolty());
			break;
		case ValueType::VALUE_TYPE_ONEOF_NOT_SET:
			break;
	}
}

void ProtoConverter::visit(DynamicByteArrayType const& _x)
{
	bool isBytes = _x.type() == DynamicByteArrayType::BYTES;
	visitType(
		isBytes ? DataType::BYTES : DataType::STRING,
		bytesArrayTypeAsString(_x),
		bytesArrayValueAsString(
			getNextCounter(),
			isBytes
		)
	);
}

// TODO: Implement struct visitor
void ProtoConverter::visit(StructType const&)
{
}

std::string ProtoConverter::arrayDimInfoAsString(ArrayDimensionInfo const& _x)
{
	return Whiskers(R"([<?isStatic><length></isStatic>])")
		("isStatic", _x.is_static())
		("length", std::to_string(getStaticArrayLengthFromFuzz(_x.length())))
		.render();
}

void ProtoConverter::arrayDimensionsAsStringVector(
	ArrayType const& _x,
	std::vector<std::string>& _vecOfStr)
{
	solAssert(_x.info_size() > 0, "Proto ABIv2 Fuzzer: Array dimensions empty.");
	for (auto const& dim: _x.info())
		_vecOfStr.push_back(arrayDimInfoAsString(dim));
}

ProtoConverter::VecOfBoolUnsigned ProtoConverter::arrayDimensionsAsPairVector(
	ArrayType const& _x
)
{
	VecOfBoolUnsigned arrayDimsPairVector = {};
	for (auto const& dim: _x.info())
		arrayDimsPairVector.push_back(arrayDimInfoAsPair(dim));
	solAssert(!arrayDimsPairVector.empty(), "Proto ABIv2 Fuzzer: Array dimensions empty.");
	return arrayDimsPairVector;
}

std::string ProtoConverter::getValueByBaseType(ArrayType const& _x)
{
	switch (_x.base_type_oneof_case())
	{
	case ArrayType::kInty:
		return integerValueAsString(isIntSigned(_x.inty()), getIntWidth(_x.inty()), getNextCounter());
	case ArrayType::kByty:
		return fixedByteValueAsString(getFixedByteWidth(_x.byty()), getNextCounter());
	case ArrayType::kAdty:
		return addressValueAsString(getNextCounter());
	case ArrayType::kBoolty:
		return boolValueAsString(getNextCounter());
	case ArrayType::kDynbytesty:
		return bytesArrayValueAsString(
			getNextCounter(),
			_x.dynbytesty().type() == DynamicByteArrayType::BYTES
		);
	// TODO: Implement structs.
	case ArrayType::kStty:
	case ArrayType::BASE_TYPE_ONEOF_NOT_SET:
		solAssert(false, "Proto ABIv2 fuzzer: Invalid array base type");
	}
}

ProtoConverter::DataType ProtoConverter::getDataTypeByBaseType(ArrayType const& _x)
{
	switch (_x.base_type_oneof_case())
	{
	case ArrayType::kInty:
	case ArrayType::kByty:
	case ArrayType::kAdty:
	case ArrayType::kBoolty:
		return DataType::VALUE;
	case ArrayType::kDynbytesty:
		return getDataTypeOfDynBytesType(_x.dynbytesty());
	case ArrayType::kStty:
	case ArrayType::BASE_TYPE_ONEOF_NOT_SET:
		solUnimplemented("Proto ABIv2 fuzzer: Invalid array base type");
	}
}

// Adds a resize operation for a given dimension of type `_type` and expression referenced
// by `_var`. `_isStatic` is true for statically sized dimensions, false otherwise.
// `_arrayLen` is equal to length of statically sized array dimension. For dynamically
// sized dimension, we use `getDynArrayLengthFromFuzz()` and a monotonically increasing
// counter to obtain actual length. Function returns dimension length.
unsigned ProtoConverter::resizeDimension(
	bool _isStatic,
	unsigned _arrayLen,
	std::string const& _var,
	std::string const& _param,
	std::string const& _type
)
{
	unsigned length;
	if (_isStatic)
		length = _arrayLen;
	else
	{
		length = getDynArrayLengthFromFuzz(_arrayLen, getNextCounter());

		// If local var, new T(l);
		// Else, l;
		std::string lhs, rhs;
		if (m_isStateVar)
		{
			lhs = _var + ".length";
			rhs = Whiskers(R"(<length>)")
				("length", std::to_string(length))
				.render();
		}
		else
		{
			lhs = _var;
			rhs = Whiskers(R"(new <type>(<length>))")
				("type", _type)
				("length", std::to_string(length))
				.render();
		}
		// If local var, x = new T(l);
		// Else, x.length = l;
		addVarDef(lhs, rhs);
	}

	// if (c.length != l)
	checkResizeOp(_param, length);
	return length;
}

void ProtoConverter::resizeHelper(
	ArrayType const& _x,
	std::vector<std::string> _arrStrVec,
	VecOfBoolUnsigned _arrInfoVec,
	std::string const& _varName,
	std::string const& _paramName
)
{
	// Initialize value expressions if we have arrived at leaf node,
	// (depth-first) recurse otherwise.
	if (_arrInfoVec.empty())
	{
		// expression name is _var
		// value is a value of base type
		std::string value = getValueByBaseType(_x);
		// add assignment and check
		DataType dataType = getDataTypeByBaseType(_x);
		addCheckedVarDef(dataType, _varName, _paramName, value);
	}
	else
	{
		auto& dim = _arrInfoVec.back();

		std::string type = std::accumulate(
			_arrStrVec.begin(),
			_arrStrVec.end(),
			std::string("")
		);
		unsigned length = resizeDimension(dim.first, dim.second, _varName, _paramName, type);
		// Recurse one level dimension down.
		_arrStrVec.pop_back();
		_arrInfoVec.pop_back();
		for (unsigned i = 0; i < length; i++)
			resizeHelper(
				_x,
				_arrStrVec,
				_arrInfoVec,
				_varName + "[" + std::to_string(i) + "]",
				_paramName + "[" + std::to_string(i) + "]"
			);
	}
}

// This function takes care of properly resizing and initializing ArrayType.
// In parallel, it adds runtime checks on array bound and values.
void ProtoConverter::resizeInitArray(
	ArrayType const& _x,
	std::string const& _baseType,
	std::string const& _varName,
	std::string const& _paramName
)
{
	VecOfBoolUnsigned arrInfoVec = arrayDimensionsAsPairVector(_x);
	std::vector<std::string> arrStrVec = {_baseType};
	arrayDimensionsAsStringVector(_x, arrStrVec);
	resizeHelper(_x, arrStrVec, arrInfoVec, _varName, _paramName);
}

// Returns array type from it's base type (e.g., int8) and array dimensions info contained in
// ArrayType.
std::string ProtoConverter::arrayTypeAsString(std::string const& _baseType, ArrayType const& _x)
{
	std::vector<std::string> typeStringVec = {_baseType};
	arrayDimensionsAsStringVector(_x, typeStringVec);

	return std::accumulate(
		typeStringVec.begin(),
		typeStringVec.end(),
		std::string("")
	);
}

void ProtoConverter::visit(ArrayType const& _x)
{
	// Bail out if input contains too few or too many dimensions.
	if (_x.info_size() == 0 || _x.info_size() > (int)s_maxArrayDimensions)
		return;

	string baseType = {};
	switch (_x.base_type_oneof_case())
	{
	case ArrayType::kInty:
		baseType = getIntTypeAsString(_x.inty());
		m_isLastParamRightPadded = false;
		break;
	case ArrayType::kByty:
		baseType = getFixedByteTypeAsString(_x.byty());
		m_isLastParamRightPadded = false;
		break;
	case ArrayType::kAdty:
		baseType = getAddressTypeAsString(_x.adty());
		m_isLastParamRightPadded = false;
		break;
	case ArrayType::kBoolty:
		baseType = getBoolTypeAsString();
		m_isLastParamRightPadded = false;
		break;
	case ArrayType::kDynbytesty:
		baseType = bytesArrayTypeAsString(_x.dynbytesty());
		m_isLastParamRightPadded = true;
		break;
	case ArrayType::kStty:
	case ArrayType::BASE_TYPE_ONEOF_NOT_SET:
		return;
	}
	visitArrayType(baseType, _x);
}

void ProtoConverter::visit(NonValueType const& _x)
{
	switch (_x.nonvalue_type_oneof_case())
	{
	case NonValueType::kDynbytearray:
		visit(_x.dynbytearray());
		break;
	case NonValueType::kArrtype:
		visit(_x.arrtype());
		break;
	case NonValueType::kStype:
		visit(_x.stype());
		break;
	case NonValueType::NONVALUE_TYPE_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(Type const& _x)
{
	switch (_x.type_oneof_case())
	{
	case Type::kVtype:
		visit(_x.vtype());
		break;
	case Type::kNvtype:
		visit(_x.nvtype());
		break;
	case Type::TYPE_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(VarDecl const& _x)
{
	visit(_x.type());
}

std::string ProtoConverter::equalityChecksAsString()
{
	return m_checks.str();
}

std::string ProtoConverter::delimiterToString(Delimiter _delimiter)
{
	switch (_delimiter)
	{
	case Delimiter::ADD:
		return ", ";
	case Delimiter::SKIP:
		return "";
	}
}

/* When a new variable is declared, we can invoke this function
 * to prepare the typed param list to be passed to callee functions.
 * We independently prepare this list for "public" and "external"
 * callee functions.
 */
void ProtoConverter::appendTypedParams(
	CalleeType _calleeType,
	bool _isValueType,
	std::string const& _typeString,
	std::string const& _varName,
	Delimiter _delimiter
)
{
	switch (_calleeType)
	{
	case CalleeType::PUBLIC:
		appendTypedParamsPublic(_isValueType, _typeString, _varName, _delimiter);
		break;
	case CalleeType::EXTERNAL:
		appendTypedParamsExternal(_isValueType, _typeString, _varName, _delimiter);
		break;
	}
}

// Adds the qualifier "calldata" to non-value parameter of an external function.
void ProtoConverter::appendTypedParamsExternal(
	bool _isValueType,
    std::string const& _typeString,
    std::string const& _varName,
    Delimiter _delimiter
)
{
	std::string qualifiedTypeString = (
		_isValueType ?
		_typeString :
		_typeString + " calldata"
	);
	m_typedParamsExternal << Whiskers(R"(<delimiter><type> <varName>)")
		("delimiter", delimiterToString(_delimiter))
		("type", qualifiedTypeString)
		("varName", _varName)
		.render();
}

// Adds the qualifier "memory" to non-value parameter of an external function.
void ProtoConverter::appendTypedParamsPublic(
	bool _isValueType,
	std::string const& _typeString,
	std::string const& _varName,
	Delimiter _delimiter
)
{
	std::string qualifiedTypeString = (
		_isValueType ?
		_typeString :
		_typeString + " memory"
		);
	m_typedParamsPublic << Whiskers(R"(<delimiter><type> <varName>)")
		("delimiter", delimiterToString(_delimiter))
		("type", qualifiedTypeString)
		("varName", _varName)
		.render();
}

std::string ProtoConverter::typedParametersAsString(CalleeType _calleeType)
{
	switch (_calleeType)
	{
	case CalleeType::PUBLIC:
		return m_typedParamsPublic.str();
	case CalleeType::EXTERNAL:
		return m_typedParamsExternal.str();
	}
}

/// Test function to be called externally.
void ProtoConverter::visit(TestFunction const& _x)
{
	m_output << R"(

	function test() public returns (uint) {
	)";

	// Define state variables in function scope
	m_output << m_statebuffer.str();

	// TODO: Support more than one but less than N local variables
	visit(_x.local_vars());

	m_output << Whiskers(R"(
		uint returnVal = this.coder_public(<parameterNames>);
		if (returnVal != 0)
			return returnVal;

		returnVal = this.coder_external(<parameterNames>);
		if (returnVal != 0)
			return uint(200000) + returnVal;

		<?atLeastOneVar>
		bytes memory argumentEncoding = abi.encode(<parameterNames>);

		returnVal = checkEncodedCall(
			this.coder_public.selector,
			argumentEncoding,
			<invalidLengthFuzz>,
			<isRightPadded>
		);
		if (returnVal != 0)
			return returnVal;

		returnVal = checkEncodedCall(
			this.coder_external.selector,
			argumentEncoding,
			<invalidLengthFuzz>,
			<isRightPadded>
		);
		if (returnVal != 0)
			return uint(200000) + returnVal;
		</atLeastOneVar>
		return 0;
	}
	)")
	("parameterNames", dev::suffixedVariableNameList(s_varNamePrefix, 0, m_varCounter))
	("invalidLengthFuzz", std::to_string(_x.invalid_encoding_length()))
	("isRightPadded", isLastParamRightPadded() ? "true" : "false")
	("atLeastOneVar", m_varCounter > 0)
	.render();
}

void ProtoConverter::writeHelperFunctions()
{
	m_output << R"(
	function bytesCompare(bytes memory a, bytes memory b) internal pure returns (bool) {
		if(a.length != b.length)
			return false;
		for (uint i = 0; i < a.length; i++)
			if (a[i] != b[i])
				return false;
		return true;
	}

	/// Accepts function selector, correct argument encoding, and length of
	/// invalid encoding and returns the correct and incorrect abi encoding
	/// for calling the function specified by the function selector.
	function createEncoding(
		bytes4 funcSelector,
		bytes memory argumentEncoding,
		uint invalidLengthFuzz,
		bool isRightPadded
	) internal pure returns (bytes memory, bytes memory)
	{
		bytes memory validEncoding = new bytes(4 + argumentEncoding.length);
		// Ensure that invalidEncoding crops at least 32 bytes (padding length
		// is at most 31 bytes) if `isRightPadded` is true.
		// This is because shorter bytes/string values (whose encoding is right
		// padded) can lead to successful decoding when fewer than 32 bytes have
		// been cropped in the worst case. In other words, if `isRightPadded` is
		// true, then
		//  0 <= invalidLength <= argumentEncoding.length - 32
		// otherwise
		//  0 <= invalidLength <= argumentEncoding.length - 1
		uint invalidLength;
		if (isRightPadded)
			invalidLength = invalidLengthFuzz % (argumentEncoding.length - 31);
		else
			invalidLength = invalidLengthFuzz % argumentEncoding.length;
		bytes memory invalidEncoding = new bytes(4 + invalidLength);
		for (uint i = 0; i < 4; i++)
			validEncoding[i] = invalidEncoding[i] = funcSelector[i];
		for (uint i = 0; i < argumentEncoding.length; i++)
			validEncoding[i+4] = argumentEncoding[i];
		for (uint i = 0; i < invalidLength; i++)
			invalidEncoding[i+4] = argumentEncoding[i];
		return (validEncoding, invalidEncoding);
	}

	/// Accepts function selector, correct argument encoding, and an invalid
	/// encoding length as input. Returns a non-zero value if either call with
	/// correct encoding fails or call with incorrect encoding succeeds.
	/// Returns zero if both calls meet expectation.
	function checkEncodedCall(
		bytes4 funcSelector,
		bytes memory argumentEncoding,
		uint invalidLengthFuzz,
		bool isRightPadded
	) public returns (uint)
	{
		(bytes memory validEncoding, bytes memory invalidEncoding) = createEncoding(
			funcSelector,
			argumentEncoding,
			invalidLengthFuzz,
			isRightPadded
		);
		(bool success, bytes memory returnVal) = address(this).call(validEncoding);
		uint returnCode = abi.decode(returnVal, (uint));
		// Return non-zero value if call fails for correct encoding
		if (success == false || returnCode != 0)
			return 400000;
		(success, ) = address(this).call(invalidEncoding);
		// Return non-zero value if call succeeds for incorrect encoding
		if (success == true)
			return 400001;
		return 0;
	}
	)";

	// These are callee functions that encode from storage, decode to
	// memory/calldata and check if decoded value matches storage value
	// return true on successful match, false otherwise
	m_output << Whiskers(R"(
	function coder_public(<parameters_memory>) public pure returns (uint) {
		<equality_checks>
		return 0;
	}

	function coder_external(<parameters_calldata>) external pure returns (uint) {
		<equality_checks>
		return 0;
	}
	)")
	("parameters_memory", typedParametersAsString(CalleeType::PUBLIC))
	("equality_checks", equalityChecksAsString())
	("parameters_calldata", typedParametersAsString(CalleeType::EXTERNAL))
	.render();
}

void ProtoConverter::visit(Contract const& _x)
{
	m_output << R"(pragma solidity >=0.0;
pragma experimental ABIEncoderV2;

contract C {
)";
	// TODO: Support more than one but less than N state variables
	visit(_x.state_vars());
	m_isStateVar = false;
	// Test function
	visit(_x.testfunction());
	writeHelperFunctions();
	m_output << "\n}";
}

string ProtoConverter::contractToString(Contract const& _input)
{
	visit(_input);
	return m_output.str();
}