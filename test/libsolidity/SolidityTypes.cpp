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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Unit tests for the type system of Solidity.
 */

#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/AST.h>
#include <libdevcore/SHA3.h>
#include <boost/test/unit_test.hpp>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SolidityTypes)

BOOST_AUTO_TEST_CASE(storage_layout_simple)
{
	MemberList members(MemberList::MemberMap({
		{string("first"), Type::fromElementaryTypeName("uint128")},
		{string("second"), Type::fromElementaryTypeName("uint120")},
		{string("wraps"), Type::fromElementaryTypeName("uint16")}
	}));
	BOOST_REQUIRE_EQUAL(u256(2), members.storageSize());
	BOOST_REQUIRE(members.memberStorageOffset("first") != nullptr);
	BOOST_REQUIRE(members.memberStorageOffset("second") != nullptr);
	BOOST_REQUIRE(members.memberStorageOffset("wraps") != nullptr);
	BOOST_CHECK(*members.memberStorageOffset("first") == make_pair(u256(0), unsigned(0)));
	BOOST_CHECK(*members.memberStorageOffset("second") == make_pair(u256(0), unsigned(16)));
	BOOST_CHECK(*members.memberStorageOffset("wraps") == make_pair(u256(1), unsigned(0)));
}

BOOST_AUTO_TEST_CASE(storage_layout_mapping)
{
	MemberList members(MemberList::MemberMap({
		{string("first"), Type::fromElementaryTypeName("uint128")},
		{string("second"), make_shared<MappingType>(
			Type::fromElementaryTypeName("uint8"),
			Type::fromElementaryTypeName("uint8")
		)},
		{string("third"), Type::fromElementaryTypeName("uint16")},
		{string("final"), make_shared<MappingType>(
			Type::fromElementaryTypeName("uint8"),
			Type::fromElementaryTypeName("uint8")
		)},
	}));
	BOOST_REQUIRE_EQUAL(u256(4), members.storageSize());
	BOOST_REQUIRE(members.memberStorageOffset("first") != nullptr);
	BOOST_REQUIRE(members.memberStorageOffset("second") != nullptr);
	BOOST_REQUIRE(members.memberStorageOffset("third") != nullptr);
	BOOST_REQUIRE(members.memberStorageOffset("final") != nullptr);
	BOOST_CHECK(*members.memberStorageOffset("first") == make_pair(u256(0), unsigned(0)));
	BOOST_CHECK(*members.memberStorageOffset("second") == make_pair(u256(1), unsigned(0)));
	BOOST_CHECK(*members.memberStorageOffset("third") == make_pair(u256(2), unsigned(0)));
	BOOST_CHECK(*members.memberStorageOffset("final") == make_pair(u256(3), unsigned(0)));
}

BOOST_AUTO_TEST_CASE(storage_layout_arrays)
{
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(1), 32).storageSize() == 1);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(1), 33).storageSize() == 2);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(2), 31).storageSize() == 2);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(7), 8).storageSize() == 2);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(7), 9).storageSize() == 3);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(31), 9).storageSize() == 9);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(32), 9).storageSize() == 9);
}

BOOST_AUTO_TEST_CASE(type_identifiers)
{
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("uint128")->identifier(), "t_uint128");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("int128")->identifier(), "t_int128");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("address")->identifier(), "t_address");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("uint8")->identifier(), "t_uint8");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("ufixed8x64")->identifier(), "t_ufixed8x64");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("fixed128x8")->identifier(), "t_fixed128x8");
	BOOST_CHECK_EQUAL(RationalNumberType(rational(7, 1)).identifier(), "t_rational_7_by_1");
	BOOST_CHECK_EQUAL(RationalNumberType(rational(200, 77)).identifier(), "t_rational_200_by_77");
	BOOST_CHECK_EQUAL(RationalNumberType(rational(2 * 200, 2 * 77)).identifier(), "t_rational_200_by_77");
	BOOST_CHECK_EQUAL(
		StringLiteralType(Literal(SourceLocation{}, Token::StringLiteral, make_shared<string>("abc - def"))).identifier(),
		 "t_stringliteral_196a9142ee0d40e274a6482393c762b16dd8315713207365e1e13d8d85b74fc4"
	);
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("bytes8")->identifier(), "t_bytes8");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("bytes32")->identifier(), "t_bytes32");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("bool")->identifier(), "t_bool");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("bytes")->identifier(), "t_bytes_storage_ptr");
	BOOST_CHECK_EQUAL(Type::fromElementaryTypeName("string")->identifier(), "t_string_storage_ptr");
	ArrayType largeintArray(DataLocation::Memory, Type::fromElementaryTypeName("int128"), u256("2535301200456458802993406410752"));
	BOOST_CHECK_EQUAL(largeintArray.identifier(), "t_int128_array2535301200456458802993406410752_memory_ptr");
	TypePointer stringArray = make_shared<ArrayType>(DataLocation::Storage, Type::fromElementaryTypeName("string"), u256("20"));
	TypePointer multiArray = make_shared<ArrayType>(DataLocation::Storage, stringArray);
	BOOST_CHECK_EQUAL(multiArray->identifier(), "t_string_storage_array20_storage_arraydyn_storage_ptr");

	ContractDefinition c(SourceLocation{}, make_shared<string>("MyContract"), {}, {}, {}, false);
	BOOST_CHECK_EQUAL(c.type()->identifier(), "t_type_t_contract_MyContract_2");
	BOOST_CHECK_EQUAL(ContractType(c, true).identifier(), "t_super_MyContract_2");

	StructDefinition s({}, make_shared<string>("Struct"), {});
	BOOST_CHECK_EQUAL(s.type()->identifier(), "t_type_t_struct_Struct_3_storage_ptr");

	EnumDefinition e({}, make_shared<string>("Enum"), {});
	BOOST_CHECK_EQUAL(e.type()->identifier(), "t_type_t_enum_Enum_4");

	TupleType t({e.type(), s.type(), stringArray, nullptr});
	BOOST_CHECK_EQUAL(t.identifier(), "t_tuple4_t_type_t_enum_Enum_4_t_type_t_struct_Struct_3_storage_ptr_t_string_storage_array20_storage_ptr_t_empty_tuple_end");

	TypePointer sha3fun = make_shared<FunctionType>(strings{}, strings{}, FunctionType::Location::SHA3);
	BOOST_CHECK_EQUAL(sha3fun->identifier(), "t_function_sha3_param0_return0_function_end");

	FunctionType metaFun(TypePointers{sha3fun}, TypePointers{s.type()});
	BOOST_CHECK_EQUAL(metaFun.identifier(), "t_function_internal_param1_t_function_sha3_param0_return0_function_end_return1_t_type_t_struct_Struct_3_storage_ptr_function_end");

	TypePointer m = make_shared<MappingType>(Type::fromElementaryTypeName("bytes32"), s.type());
	MappingType m2(Type::fromElementaryTypeName("uint64"), m);
	BOOST_CHECK_EQUAL(m2.identifier(), "t_mapping_t_uint64_to_t_mapping_t_bytes32_to_t_type_t_struct_Struct_3_storage_ptr_mapping_end_mapping_end");

	// TypeType is tested with contract

	auto emptyParams = make_shared<ParameterList>(SourceLocation(), std::vector<ASTPointer<VariableDeclaration>>());
	ModifierDefinition mod(SourceLocation{}, make_shared<string>("modif"), {}, emptyParams, {});
	BOOST_CHECK_EQUAL(ModifierType(mod).identifier(), "t_modifier_param0_end_modifier");

	SourceUnit su({}, {});
	BOOST_CHECK_EQUAL(ModuleType(su).identifier(), "t_module_7");
	BOOST_CHECK_EQUAL(MagicType(MagicType::Kind::Block).identifier(), "t_magic_block");
	BOOST_CHECK_EQUAL(MagicType(MagicType::Kind::Message).identifier(), "t_magic_message");
	BOOST_CHECK_EQUAL(MagicType(MagicType::Kind::Transaction).identifier(), "t_magic_transaction");

	BOOST_CHECK_EQUAL(InaccessibleDynamicType().identifier(), "t_inaccessible");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
