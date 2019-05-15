#include <test/tools/ossfuzz/protoToAbiV2.h>

using namespace dev::test::abiv2fuzzer;
using namespace std;

void ProtoConverter::visit(SignedIntegerType const& _x)
{
	m_output << "sint" << std::to_string(8 * ((_x.width() % 32) + 1));
}

void ProtoConverter::visit(UnsignedIntegerType const& _x)
{
	m_output << "uint" << std::to_string(8 * ((_x.width() % 32) + 1));
}

void ProtoConverter::visit(SignedIntegerValue const& _x)
{
	auto width = 8 * ((_x.width() % 32) + 1);
	s256 value = generateSigned(_x.value64(), _x.value128(), _x.value192(), _x.value256());
	switch (width)
	{
	case 8:
		m_output << s8(value);
		break;
	case 16:
		m_output << s16(value);
		break;
	case 24:
		m_output << s24(value);
		break;
	case 32:
		m_output << s32(value);
		break;
	case 40:
		m_output << s40(value);
		break;
	case 48:
		m_output << s48(value);
		break;
	case 56:
		m_output << s56(value);
		break;
	case 64:
		m_output << s64(value);
		break;
	case 72:
		m_output << s72(value);
		break;
	case 80:
		m_output << s80(value);
		break;
	case 88:
		m_output << s88(value);
		break;
	case 96:
		m_output << s96(value);
		break;
	case 104:
		m_output << s104(value);
		break;
	case 112:
		m_output << s112(value);
		break;
	case 120:
		m_output << s120(value);
		break;
	case 128:
		m_output << s128(value);
		break;
	case 136:
		m_output << s136(value);
		break;
	case 144:
		m_output << s144(value);
		break;
	case 152:
		m_output << s152(value);
		break;
	case 160:
		m_output << s160(value);
		break;
	case 168:
		m_output << s168(value);
		break;
	case 176:
		m_output << s176(value);
		break;
	case 184:
		m_output << s184(value);
		break;
	case 192:
		m_output << s192(value);
		break;
	case 200:
		m_output << s200(value);
		break;
	case 208:
		m_output << s208(value);
		break;
	case 216:
		m_output << s216(value);
		break;
	case 224:
		m_output << s224(value);
		break;
	case 232:
		m_output << s232(value);
		break;
	case 240:
		m_output << s240(value);
		break;
	case 248:
		m_output << s248(value);
		break;
	case 256:
		m_output << s256(value);
		break;
	}
}

void ProtoConverter::visit(UnsignedIntegerValue const& _x)
{
	auto width = 8 * ((_x.width() % 32) + 1);
	u256 value = generateUnsigned(_x.value64(), _x.value128(), _x.value192(), _x.value256());
	switch (width)
	{
	case 8:
		m_output << u8(value);
		break;
	case 16:
		m_output << u16(value);
		break;
	case 24:
		m_output << u24(value);
		break;
	case 32:
		m_output << u32(value);
		break;
	case 40:
		m_output << u40(value);
		break;
	case 48:
		m_output << u48(value);
		break;
	case 56:
		m_output << u56(value);
		break;
	case 64:
		m_output << u64(value);
		break;
	case 72:
		m_output << u72(value);
		break;
	case 80:
		m_output << u80(value);
		break;
	case 88:
		m_output << u88(value);
		break;
	case 96:
		m_output << u96(value);
		break;
	case 104:
		m_output << u104(value);
		break;
	case 112:
		m_output << u112(value);
		break;
	case 120:
		m_output << u120(value);
		break;
	case 128:
		m_output << u128(value);
		break;
	case 136:
		m_output << u136(value);
		break;
	case 144:
		m_output << u144(value);
		break;
	case 152:
		m_output << u152(value);
		break;
	case 160:
		m_output << u160(value);
		break;
	case 168:
		m_output << u168(value);
		break;
	case 176:
		m_output << u176(value);
		break;
	case 184:
		m_output << u184(value);
		break;
	case 192:
		m_output << u192(value);
		break;
	case 200:
		m_output << u200(value);
		break;
	case 208:
		m_output << u208(value);
		break;
	case 216:
		m_output << u216(value);
		break;
	case 224:
		m_output << u224(value);
		break;
	case 232:
		m_output << u232(value);
		break;
	case 240:
		m_output << u240(value);
		break;
	case 248:
		m_output << u248(value);
		break;
	case 256:
		m_output << u256(value);
		break;
	}
}

// Value is enclosed within double quote e.g., "hello"
void ProtoConverter::visit(FixedByteArrayValue const& _x)
{
	// RHS can have fewer bytes but never more than type
	unsigned numRequiredBytes = (_x.width() % 32) + 1;
	auto numAvailableBytes = static_cast<unsigned>(_x.value().size());
	auto usedBytes = min(numRequiredBytes, numAvailableBytes);
	m_output << "\"" << _x.value().substr(0, usedBytes) << "\"";
}

void ProtoConverter::visit(DynamicByteType const&)
{
	m_output << "bytes";
}

void ProtoConverter::visit(DynamicStringType const&)
{
	m_output << "string";
}

void ProtoConverter::visit(DynamicByteValue const& _x)
{
	m_output << "\"" << hex << _x.value() << "\"";
}

void ProtoConverter::visit(DynamicStringValue const& _x)
{
	m_output << "\"" << _x.value() << "\"";
}

void ProtoConverter::visit(AddressValue const&)
{

}

void ProtoConverter::visit(Type const& _x)
{
	switch (_x.type_oneof_case())
	{
	case Type::kStype:
		visit(_x.stype());
		break;
	case Type::kDtype:
		visit(_x.dtype());
		break;
	case Type::TYPE_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(VarDecl const& _x)
{
	visit(_x.type());
}

void ProtoConverter::visit(Statement const& _x)
{
	switch (_x.statement_oneof_case())
	{
	case Statement::kDecl:
		visit(_x.decl());
		break;
	case Statement::kAssignment:
		visit(_x.assignment());
		break;
	case Statement::kStructdef:
		visit(_x.structdef());
		break;
	case Statement::STATEMENT_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(Assignment const&)
{

}

void ProtoConverter::visit(StaticType const& _x)
{
	switch (_x.static_type_oneof_case())
	{
	case StaticType::kInteger:
		visit(_x.integer());
		break;
	case StaticType::kFbarray:
		visit(_x.fbarray());
		break;
	case StaticType::kAddress:
		visit(_x.address());
		break;
	case StaticType::kFsarray:
		visit(_x.fsarray());
		break;
	case StaticType::STATIC_TYPE_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(StructType const&)
{

}

void ProtoConverter::visit(AddressType const& _x)
{
	switch (_x.atype())
	{
	case AddressType::ADDRESS:
		m_output << "address";
		break;
	case AddressType::PAYABLE:
		m_output << "address payable";
		break;
	}
}

void ProtoConverter::visit(DynamicType const& _x)
{
	switch (_x.dynamic_type_oneof_case())
	{
	case DynamicType::kStructtype:
		visit(_x.structtype());
		break;
	case DynamicType::kDynbytearray:
		visit(_x.dynbytearray());
		break;
	case DynamicType::DYNAMIC_TYPE_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(IntegerType const& _x)
{
	switch (_x.integer_type_case())
	{
	case IntegerType::kSint:
		visit(_x.sint());
		break;
	case IntegerType::kUint:
		visit(_x.uint());
		break;
	case IntegerType::INTEGER_TYPE_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(StructTypeDefinition const& _x)
{
	m_output << "struct S_" << m_numStructs++ << " {";
	for (auto const& m: _x.t())
		visit(m);
	m_output << " }\n";
}

void ProtoConverter::visit(FixedByteArrayType const& _x)
{
	unsigned numBytes = (_x.width() % 32) + 1;
	m_output << "bytes" << numBytes;
}

void ProtoConverter::visit(DynamicByteArrayType const& _x)
{
	switch (_x.dynamic_byte_oneof_case())
	{
	case DynamicByteArrayType::kByte:
		visit(_x.byte());
		break;
	case DynamicByteArrayType::kString:
		visit(_x.string());
		break;
	case DynamicByteArrayType::DYNAMIC_BYTE_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(FixedSizeArrayType const& _x)
{
	visit(_x.type());
	m_output << "[" << _x.size() << "]";
}

// Callee function
void ProtoConverter::visit(CoderFunction const&)
{
	// TODO: We should also mix memory and calldata here.
	// function g(uint a, uint16 b, S memory c, bool[] memory d, uint16[2] memory e)
	m_output << "function g(";
	// TODO: Print input parameters
	m_output << ") ";
	//   returns (uint, uint16, S memory, bool[] memory, uint16[2] memory) {
	m_output << "returns (";
	// TODO: Print input parameter types
	m_output << ") {\n";
	// TODO: Return tuple containing input parameter names
	//    return (a, b, c, d, e);
	m_output << "return (";
	m_output << ");\n";
	m_output << "}\n";
}

// Caller function
void ProtoConverter::visit(TestFunction const& _x)
{
	// Function definition
	//  function f() returns (bool) {
	m_output << "function f() returns (bool) {\n";

	// Var decls
	//    uint w = 3482374;
	// Assignments
	//    x.length = 3;
	//    x[0] = true;
	//    x[1] = false;
	//    x[2] = true;
	//    y = 8;
	//    s.u = 7;
	//    s.v = -3;
	//    uint16[2] memory e;
	//    e[0] = 7;
	//    e[1] = 20;
	for (auto const& s: _x.statements())
		visit(s);

	// TODO: Make a call to g()
	//    // Some of the arguments are read from the stack / from memory,
	//    // some from storage.
	//    (uint a1, uint16 b1, S memory c1, bool[] memory d1, uint16[2] memory e1) = this.g(w, y, s, x, e);

	// TODO: Add assertions
	//    if (a1 != w) return false;
	//    if (b1 != y) return false;
	//    if (c1.u != s.u) return false;
	//    if (c1.v != s.v) return false;
	//    if (d1.length != x.length) return false;
	//    if (d1[0] != x[0]) return false;
	//    if (d1[1] != x[1]) return false;
	//    if (d1[2] != x[2]) return false;
	//    if (e1[0] != e[0]) return false;
	//    if (e1[1] != e[1]) return false;


	//    return true;
	//  }
	m_output << "return true;\n";
	m_output << "}\n";
}

void ProtoConverter::visit(ContractStatement const& _x)
{
	switch (_x.contract_stmt_oneof_case())
	{
	case ContractStatement::kDecl:
		visit(_x.decl());
		break;
	case ContractStatement::kStructdef:
		visit(_x.structdef());
		break;
	case ContractStatement::CONTRACT_STMT_ONEOF_NOT_SET:
		break;
	}
}

void ProtoConverter::visit(Program const& _x)
{
	// Pragmas
	m_output << "pragma experimental ABIEncoderV2;\n";
	// Contract definition
	m_output << "contract C {\n";
	// Storage vars
	// Dynamic types
	//  struct S { uint16 u; int8 v; }
	// Var decls (mix of static and dynamic types)
	//  bool[] x;
	//  uint16 y;
	//  S s;
	for (auto const& cs: _x.cstatements())
		visit(cs);
	// Test function
//	visit(_x.testfunction());
	// Coder function
//	visit(_x.coderfunction());
	m_output << "}\n";
}

string ProtoConverter::programToString(Program const& _input)
{
	visit(_input);
	return m_output.str();
}