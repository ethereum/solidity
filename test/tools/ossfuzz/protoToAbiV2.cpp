#include <test/tools/ossfuzz/protoToAbiV2.h>

using namespace std;
using namespace dev;
using namespace dev::test::abiv2fuzzer;

string ProtoConverter::appendVarDeclToOutput(
	string const& _type,
	string const& _varName,
	string const& _qualifier
)
{
	// One level of indentation for state variable declarations
	// Two levels of indentation for local variable declarations
	return Whiskers(R"(
	<?isLocalVar>	</isLocalVar><type><?qual> <qualifier></qual> <varName>;)"
		)
		("isLocalVar", !m_isStateVar)
		("type", _type)
		("qual", !_qualifier.empty())
		("qualifier", _qualifier)
		("varName", _varName)
		.render() +
		"\n";
}

pair<string, string> ProtoConverter::visit(Type const& _type)
{
	switch (_type.type_oneof_case())
	{
	case Type::kVtype:
		return visit(_type.vtype());
	case Type::kNvtype:
		return visit(_type.nvtype());
	case Type::TYPE_ONEOF_NOT_SET:
		return make_pair("", "");
	}
}

pair<string, string> ProtoConverter::visit(ValueType const& _type)
{
	switch (_type.value_type_oneof_case())
	{
	case ValueType::kBoolty:
		return visit(_type.boolty());
	case ValueType::kInty:
		return visit(_type.inty());
	case ValueType::kByty:
		return visit(_type.byty());
	case ValueType::kAdty:
		return visit(_type.adty());
	case ValueType::VALUE_TYPE_ONEOF_NOT_SET:
		return make_pair("", "");
	}
}

pair<string, string> ProtoConverter::visit(NonValueType const& _type)
{
	switch (_type.nonvalue_type_oneof_case())
	{
	case NonValueType::kDynbytearray:
		return visit(_type.dynbytearray());
	case NonValueType::kArrtype:
		if (ValidityVisitor().visit(_type.arrtype()))
			return visit(_type.arrtype());
		else
			return make_pair("", "");
	case NonValueType::kStype:
		if (ValidityVisitor().visit(_type.stype()))
			return visit(_type.stype());
		else
			return make_pair("", "");
	case NonValueType::NONVALUE_TYPE_ONEOF_NOT_SET:
		return make_pair("", "");
	}
}

pair<string, string> ProtoConverter::visit(BoolType const& _type)
{
	return processType(_type, true);
}

pair<string, string> ProtoConverter::visit(IntegerType const& _type)
{
	return processType(_type, true);
}

pair<string, string> ProtoConverter::visit(FixedByteType const& _type)
{
	return processType(_type, true);
}

pair<string, string> ProtoConverter::visit(AddressType const& _type)
{
	return processType(_type, true);
}

pair<string, string> ProtoConverter::visit(DynamicByteArrayType const& _type)
{
	return processType(_type, false);
}

pair<string, string> ProtoConverter::visit(ArrayType const& _type)
{
	return processType(_type, false);
}

pair<string, string> ProtoConverter::visit(StructType const& _type)
{
	return processType(_type, false);
}

template <typename T>
pair<string, string> ProtoConverter::processType(T const& _type, bool _isValueType)
{
	ostringstream local, global;
	auto varNames = newVarNames(getNextVarCounter());
	string varName = varNames.first;
	string paramName = varNames.second;
	string location{};
	if (!m_isStateVar && !_isValueType)
		location = "memory";

	auto varDeclBuffers = varDecl(
		varName,
		paramName,
		_type,
		_isValueType,
		location
	);
	global << varDeclBuffers.first;
	local << varDeclBuffers.second;
	auto assignCheckBuffers = assignChecker(varName, paramName, _type);
	global << assignCheckBuffers.first;
	local << assignCheckBuffers.second;

	m_structCounter += m_numStructsAdded;
	return make_pair(global.str(), local.str());
}

template <typename T>
pair<string, string> ProtoConverter::varDecl(
	string const& _varName,
	string const& _paramName,
	T _type,
	bool _isValueType,
	string const& _location
)
{
	ostringstream local, global;

	TypeVisitor tVisitor(m_structCounter);
	string typeStr = tVisitor.visit(_type);
	if (typeStr.empty())
		return make_pair("", "");

	// Append struct defs
	global << tVisitor.structDef();
	m_numStructsAdded = tVisitor.numStructs();

	// variable declaration
	if (m_isStateVar)
		global << appendVarDeclToOutput(typeStr, _varName, _location);
	else
		local << appendVarDeclToOutput(typeStr, _varName, _location);

	// Add typed params for calling public and external functions with said type
	appendTypedParams(
		CalleeType::PUBLIC,
		_isValueType,
		typeStr,
		_paramName,
		((m_varCounter == 1) ? Delimiter::SKIP : Delimiter::ADD)
	);
	appendTypedParams(
		CalleeType::EXTERNAL,
		_isValueType,
		typeStr,
		_paramName,
		((m_varCounter == 1) ? Delimiter::SKIP : Delimiter::ADD)
	);

	// Update dyn param only if necessary
	if (tVisitor.isLastDynParamRightPadded())
		m_isLastDynParamRightPadded = true;

	return make_pair(global.str(), local.str());
}

template <typename T>
pair<string, string> ProtoConverter::assignChecker(
	string const& _varName,
	string const& _paramName,
	T _type
)
{
	ostringstream local;
	AssignCheckVisitor acVisitor(
		_varName,
		_paramName,
		m_returnValue,
		m_isStateVar,
		m_counter,
		m_structCounter
	);
	pair<string, string> assignCheckStrPair = acVisitor.visit(_type);
	m_returnValue += acVisitor.errorStmts();
	m_counter += acVisitor.counted();

	m_checks << assignCheckStrPair.second;

	// State variables cannot be assigned in contract-scope
	// Therefore, we buffer their assignments and
	// render them in function scope later.
	local << assignCheckStrPair.first;
	return make_pair("", local.str());
}

pair<string, string> ProtoConverter::visit(VarDecl const& _x)
{
	return visit(_x.type());
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
string ProtoConverter::visit(TestFunction const& _x, string const& _storageVarDefs)
{
	// TODO: Support more than one but less than N local variables
	auto localVarBuffers = visit(_x.local_vars());

	string structTypeDecl = localVarBuffers.first;
	string localVarDefs = localVarBuffers.second;

	ostringstream testBuffer;
	string functionDecl = "function test() public returns (uint)";
	testBuffer << Whiskers(R"(<structTypeDecl>
	<functionDecl> {
<storageVarDefs>
<localVarDefs>
<testCode>
	})")
		("structTypeDecl", structTypeDecl)
		("functionDecl", functionDecl)
		("storageVarDefs", _storageVarDefs)
		("localVarDefs", localVarDefs)
		("testCode", testCode(_x.invalid_encoding_length()))
		.render();
	return testBuffer.str();
}

string ProtoConverter::testCode(unsigned _invalidLength)
{
	return Whiskers(R"(
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
		)")
		("parameterNames", dev::suffixedVariableNameList(s_varNamePrefix, 0, m_varCounter))
		("invalidLengthFuzz", std::to_string(_invalidLength))
		("isRightPadded", isLastDynParamRightPadded() ? "true" : "false")
		("atLeastOneVar", m_varCounter > 0)
		.render();
}

string ProtoConverter::helperFunctions()
{
	stringstream helperFuncs;
	helperFuncs << R"(
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
	helperFuncs << Whiskers(R"(
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
	return helperFuncs.str();
}

void ProtoConverter::visit(Contract const& _x)
{
	string pragmas = R"(pragma solidity >=0.0;
pragma experimental ABIEncoderV2;)";

	// TODO: Support more than one but less than N state variables
	auto storageBuffers = visit(_x.state_vars());
	string storageVarDecls = storageBuffers.first;
	string storageVarDefs = storageBuffers.second;
	m_isStateVar = false;
	string testFunction = visit(_x.testfunction(), storageVarDefs);
	/* Structure of contract body
	 * - Storage variable declarations
	 * - Struct type declarations
	 * - Test function
     *     - Storage variable definitions
	 *     - Local variable definitions
	 *     - Test code proper (calls public and external functions)
	 * - Helper functions
	 */
	ostringstream contractBody;
	contractBody << storageVarDecls
	             << testFunction
	             << helperFunctions();
	m_output << Whiskers(R"(<pragmas>
<contractStart>
<contractBody>
<contractEnd>)")
		("pragmas", pragmas)
		("contractStart", "contract C {")
		("contractBody", contractBody.str())
		("contractEnd", "}")
		.render();
}

string ProtoConverter::contractToString(Contract const& _input)
{
	visit(_input);
	return m_output.str();
}

/// Type visitor
string TypeVisitor::visit(BoolType const&)
{
	m_baseType = "bool";
	return m_baseType;
}

string TypeVisitor::visit(IntegerType const& _type)
{
	m_baseType = getIntTypeAsString(_type);
	return m_baseType;
}

string TypeVisitor::visit(FixedByteType const& _type)
{
	m_baseType = getFixedByteTypeAsString(_type);
	return m_baseType;
}

string TypeVisitor::visit(AddressType const& _type)
{
	m_baseType = getAddressTypeAsString(_type);
	return m_baseType;
}

string TypeVisitor::visit(ArrayType const& _type)
{
	if (!ValidityVisitor().visit(_type))
		return "";
	string baseType = visit(_type.t());
	solAssert(!baseType.empty(), "");
	string arrayBraces = _type.is_static() ?
	                     string("[") +
	                     to_string(getStaticArrayLengthFromFuzz(_type.length())) +
	                     string("]") :
	                     string("[]");
	m_baseType += arrayBraces;
	if (!_type.is_static())
		m_isLastDynParamRightPadded = true;
	return baseType + arrayBraces;
}

string TypeVisitor::visit(DynamicByteArrayType const& _type)
{
	m_isLastDynParamRightPadded = true;
	m_baseType = bytesArrayTypeAsString(_type);
	return m_baseType;
}

void TypeVisitor::structDefinition(StructType const& _type)
{
	// Return an empty string if struct is empty
	solAssert(ValidityVisitor().visit(_type), "");

	// Reset field counter and indentation
	unsigned wasFieldCounter = m_structFieldCounter;
	unsigned wasIndentation = m_indentation;

	m_indentation = 1;
	m_structFieldCounter = 0;

	// Commence struct declaration
	string structDef = lineString(
		"struct " +
		string(s_structNamePrefix) +
		to_string(m_structCounter) +
		" {"
	);

	// Increase indentation for struct fields
	m_indentation++;
	for (auto const& t: _type.t())
	{
		string type{};

		if (!ValidityVisitor().visit(t))
			continue;

		TypeVisitor tVisitor(m_structCounter + 1);
		type = tVisitor.visit(t);
		m_structCounter += tVisitor.numStructs();
		m_structDef << tVisitor.structDef();

		solAssert(!type.empty(), "");

		structDef += lineString(
			Whiskers(R"(<type> <member>;)")
				("type", type)
				("member", "m" + to_string(m_structFieldCounter++))
				.render()
		);
	}
	m_indentation--;
	structDef += lineString("}");
	m_structCounter++;
	m_structDef << structDef;
	m_indentation = wasIndentation;
	m_structFieldCounter = wasFieldCounter;
}

string TypeVisitor::visit(StructType const& _type)
{
	if (ValidityVisitor().visit(_type))
	{
		// Add struct definition
		structDefinition(_type);
		// Set last dyn param if struct contains a dyn param e.g., bytes, array etc.
		m_isLastDynParamRightPadded = DynParamVisitor().visit(_type);
		// If top-level struct is a non-emtpy struct, assign the name S<suffix>
		m_baseType = s_structTypeName + to_string(m_structStartCounter);
	}
	else
		m_baseType = {};

	return m_baseType;
}

/// AssignCheckVisitor implementation
pair<string, string> AssignCheckVisitor::visit(BoolType const& _type)
{
	string value = ValueGetterVisitor(counter()).visit(_type);
	return assignAndCheckStringPair(m_varName, m_paramName, value, value, DataType::VALUE);
}

pair<string, string> AssignCheckVisitor::visit(IntegerType const& _type)
{
	string value = ValueGetterVisitor(counter()).visit(_type);
	return assignAndCheckStringPair(m_varName, m_paramName, value, value, DataType::VALUE);
}

pair<string, string> AssignCheckVisitor::visit(FixedByteType const& _type)
{
	string value = ValueGetterVisitor(counter()).visit(_type);
	return assignAndCheckStringPair(m_varName, m_paramName, value, value, DataType::VALUE);
}

pair<string, string> AssignCheckVisitor::visit(AddressType const& _type)
{
	string value = ValueGetterVisitor(counter()).visit(_type);
	return assignAndCheckStringPair(m_varName, m_paramName, value, value, DataType::VALUE);
}

pair<string, string> AssignCheckVisitor::visit(DynamicByteArrayType const& _type)
{
	string value = ValueGetterVisitor(counter()).visit(_type);
	DataType dataType = _type.type() == DynamicByteArrayType::BYTES ? DataType::BYTES :	DataType::STRING;
	return assignAndCheckStringPair(m_varName, m_paramName, value, value, dataType);
}

pair<string, string> AssignCheckVisitor::visit(ArrayType const& _type)
{
	if (!ValidityVisitor().visit(_type))
		return make_pair("", "");

	// Obtain type of array to be resized and initialized
	string typeStr{};

	unsigned wasStructCounter = m_structCounter;
	TypeVisitor tVisitor(m_structCounter);
	typeStr = tVisitor.visit(_type);

	pair<string, string> resizeBuffer;
	string lengthStr;
	unsigned length;

	// Resize dynamic arrays
	if (!_type.is_static())
	{
		length = getDynArrayLengthFromFuzz(_type.length(), counter());
		lengthStr = to_string(length);
		if (m_stateVar)
			resizeBuffer = assignAndCheckStringPair(
				m_varName + ".length",
				m_paramName + ".length",
				lengthStr,
				lengthStr,
				DataType::VALUE
				);
		else
		{
			// Resizing memory arrays via the new operator
			string resizeOp = Whiskers(R"(new <fullTypeStr>(<length>))")
				("fullTypeStr", typeStr)
				("length", lengthStr)
				.render();
			resizeBuffer = assignAndCheckStringPair(
				m_varName,
				m_paramName + ".length",
				resizeOp,
				lengthStr,
				DataType::VALUE
				);
		}
	}
	else
	{
		length = getStaticArrayLengthFromFuzz(_type.length());
		lengthStr = to_string(length);
		// Add check on length
		resizeBuffer.second = checkString(m_paramName + ".length", lengthStr, DataType::VALUE);
	}

	// Add assignCheckBuffer and check statements
	pair<string, string> assignCheckBuffer;
	string wasVarName = m_varName;
	string wasParamName = m_paramName;
	for (unsigned i = 0; i < length; i++)
	{
		m_varName = wasVarName + "[" + to_string(i) + "]";
		m_paramName = wasParamName + "[" + to_string(i) + "]";
		pair<string, string> assign = visit(_type.t());
		assignCheckBuffer.first += assign.first;
		assignCheckBuffer.second += assign.second;
		if (i < length - 1)
			m_structCounter = wasStructCounter;
	}

	// Since struct visitor won't be called for zero-length
	// arrays, struct counter will not get incremented. Therefore,
	// we need to manually force a recursive struct visit.
	if (length == 0 && TypeVisitor().arrayOfStruct(_type))
		visit(_type.t());

	m_varName = wasVarName;
	m_paramName = wasParamName;

	// Compose resize and initialization assignment and check
	return make_pair(
		resizeBuffer.first + assignCheckBuffer.first,
		resizeBuffer.second + assignCheckBuffer.second
	);
}

pair<string, string> AssignCheckVisitor::visit(StructType const& _type)
{
	if (!ValidityVisitor().visit(_type))
		return make_pair("", "");

	pair<string, string> assignCheckBuffer;
	unsigned i = 0;

	// Increment struct counter
	m_structCounter++;

	string wasVarName = m_varName;
	string wasParamName = m_paramName;

	for (auto const& t: _type.t())
	{
		m_varName = wasVarName + ".m" + to_string(i);
		m_paramName = wasParamName + ".m" + to_string(i);
		pair<string, string> assign = visit(t);
		// If type is not well formed continue without
		// updating state.
		if (assign.first.empty() && assign.second.empty())
			continue;
		assignCheckBuffer.first += assign.first;
		assignCheckBuffer.second += assign.second;
		i++;
	}
	m_varName = wasVarName;
	m_paramName = wasParamName;
	return assignCheckBuffer;
}

pair<string, string> AssignCheckVisitor::assignAndCheckStringPair(
	string const& _varRef,
	string const& _checkRef,
	string const& _assignValue,
	string const& _checkValue,
	DataType _type
)
{
	return make_pair(assignString(_varRef, _assignValue), checkString(_checkRef, _checkValue, _type));
}

string AssignCheckVisitor::assignString(string const& _ref, string const& _value)
{
	string assignStmt = Whiskers(R"(<ref> = <value>;)")
		("ref", _ref)
		("value", _value)
		.render();
	return indentation() + assignStmt + "\n";
}

string AssignCheckVisitor::checkString(string const& _ref, string const& _value, DataType _type)
{
	string checkPred;
	switch (_type)
	{
	case DataType::STRING:
		checkPred = Whiskers(R"(!bytesCompare(bytes(<varName>), <value>))")
			("varName", _ref)
			("value", _value)
			.render();
		break;
	case DataType::BYTES:
		checkPred = Whiskers(R"(!bytesCompare(<varName>, <value>))")
			("varName", _ref)
			("value", _value)
			.render();
		break;
	case DataType::VALUE:
		checkPred = Whiskers(R"(<varName> != <value>)")
			("varName", _ref)
			("value", _value)
			.render();
		break;
	case DataType::ARRAY:
		solUnimplemented("Proto ABIv2 fuzzer: Invalid data type.");
	}
	string checkStmt = Whiskers(R"(if (<checkPred>) return <errCode>;)")
		("checkPred", checkPred)
		("errCode", to_string(m_errorCode++))
		.render();
	return indentation() + checkStmt + "\n";
}

/// ValueGetterVisitor
string ValueGetterVisitor::visit(BoolType const&)
{
	return counter() % 2 ? "true" : "false";
}

string ValueGetterVisitor::visit(IntegerType const& _type)
{
	return integerValueAsString(
		_type.is_signed(),
		getIntWidth(_type),
		counter()
	);
}

string ValueGetterVisitor::visit(FixedByteType const& _type)
{
	return fixedByteValueAsString(
		getFixedByteWidth(_type),
		counter()
	);
}

string ValueGetterVisitor::visit(AddressType const&)
{
	return addressValueAsString(counter());
}

string ValueGetterVisitor::visit(DynamicByteArrayType const& _type)
{
	return bytesArrayValueAsString(
		counter(),
		getDataTypeOfDynBytesType(_type) == DataType::BYTES
	);
}

std::string ValueGetterVisitor::integerValueAsString(bool _sign, unsigned _width, unsigned _counter)
{
	if (_sign)
		return intValueAsString(_width, _counter);
	else
		return uintValueAsString(_width, _counter);
}

/* Input(s)
 *   - Unsigned integer to be hashed
 *   - Width of desired uint value
 * Processing
 *   - Take hash of first parameter and mask it with the max unsigned value for given bit width
 * Output
 *   - string representation of uint value
 */
std::string ValueGetterVisitor::uintValueAsString(unsigned _width, unsigned _counter)
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
std::string ValueGetterVisitor::intValueAsString(unsigned _width, unsigned _counter)
{
	solAssert(
		(_width % 8 == 0),
		"Proto ABIv2 Fuzzer: Signed integer width is not a multiple of 8"
	);
	return maskUnsignedIntToHex(_counter, ((_width/4) - 1));
}

std::string ValueGetterVisitor::croppedString(
	unsigned _numBytes,
	unsigned _counter,
	bool _isHexLiteral
)
{
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

std::string ValueGetterVisitor::hexValueAsString(
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

std::string ValueGetterVisitor::fixedByteValueAsString(unsigned _width, unsigned _counter)
{
	solAssert(
		(_width >= 1 && _width <= 32),
		"Proto ABIv2 Fuzzer: Fixed byte width is not between 1--32"
	);
	return hexValueAsString(_width, _counter, /*isHexLiteral=*/true);
}

std::string ValueGetterVisitor::addressValueAsString(unsigned _counter)
{
	return Whiskers(R"(address(<value>))")
		("value", uintValueAsString(160, _counter))
		.render();
}

std::string ValueGetterVisitor::variableLengthValueAsString(
	unsigned _numBytes,
	unsigned _counter,
	bool _isHexLiteral
)
{
	// TODO: Move this to caller
//	solAssert(_numBytes >= 0 && _numBytes <= s_maxDynArrayLength,
//	          "Proto ABIv2 fuzzer: Invalid hex length"
//	);
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

string ValueGetterVisitor::bytesArrayValueAsString(unsigned _counter, bool _isHexLiteral)
{
	return variableLengthValueAsString(
		getVarLength(_counter),
		_counter,
		_isHexLiteral
	);
}