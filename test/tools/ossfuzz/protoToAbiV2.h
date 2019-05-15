#pragma once

#include <ostream>
#include <sstream>
#include <test/tools/ossfuzz/abiV2Proto.pb.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/number.hpp>

template <unsigned numBits>
using uFixedNum = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<numBits, numBits, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;
using u8 = uFixedNum<8>;
using u16 = uFixedNum<16>;
using u24 = uFixedNum<24>;
using u32 = uFixedNum<32>;
using u40 = uFixedNum<40>;
using u48 = uFixedNum<48>;
using u56 = uFixedNum<56>;
using u64 = uFixedNum<64>;
using u72 = uFixedNum<72>;
using u80 = uFixedNum<80>;
using u88 = uFixedNum<88>;
using u96 = uFixedNum<96>;
using u104 = uFixedNum<104>;
using u112 = uFixedNum<112>;
using u120 = uFixedNum<120>;
using u128 = uFixedNum<128>;
using u136 = uFixedNum<136>;
using u144 = uFixedNum<144>;
using u152 = uFixedNum<152>;
using u160 = uFixedNum<160>;
using u168 = uFixedNum<168>;
using u176 = uFixedNum<176>;
using u184 = uFixedNum<184>;
using u192 = uFixedNum<192>;
using u200 = uFixedNum<200>;
using u208 = uFixedNum<208>;
using u216 = uFixedNum<216>;
using u224 = uFixedNum<224>;
using u232 = uFixedNum<232>;
using u240 = uFixedNum<240>;
using u248 = uFixedNum<248>;
using u256 = uFixedNum<256>;

template <unsigned numBits>
using sFixedNum = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<numBits, numBits, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void>>;
using s8 = sFixedNum<8>;
using s16 = sFixedNum<16>;
using s24 = sFixedNum<24>;
using s32 = sFixedNum<32>;
using s40 = sFixedNum<40>;
using s48 = sFixedNum<48>;
using s56 = sFixedNum<56>;
using s64 = sFixedNum<64>;
using s72 = sFixedNum<72>;
using s80 = sFixedNum<80>;
using s88 = sFixedNum<88>;
using s96 = sFixedNum<96>;
using s104 = sFixedNum<104>;
using s112 = sFixedNum<112>;
using s120 = sFixedNum<120>;
using s128 = sFixedNum<128>;
using s136 = sFixedNum<136>;
using s144 = sFixedNum<144>;
using s152 = sFixedNum<152>;
using s160 = sFixedNum<160>;
using s168 = sFixedNum<168>;
using s176 = sFixedNum<176>;
using s184 = sFixedNum<184>;
using s192 = sFixedNum<192>;
using s200 = sFixedNum<200>;
using s208 = sFixedNum<208>;
using s216 = sFixedNum<216>;
using s224 = sFixedNum<224>;
using s232 = sFixedNum<232>;
using s240 = sFixedNum<240>;
using s248 = sFixedNum<248>;
using s256 = sFixedNum<256>;

namespace dev
{
namespace test
{
namespace abiv2fuzzer
{
class ProtoConverter
{
public:
	ProtoConverter()
	{
		m_numStructs = 0;
	}
	ProtoConverter(ProtoConverter const&) = delete;
	ProtoConverter(ProtoConverter&&) = delete;
	std::string programToString(Program const& _input);

private:
	void visit(IntegerType const&);
	void visit(SignedIntegerType const&);
	void visit(UnsignedIntegerType const&);
	void visit(SignedIntegerValue const&);
	void visit(UnsignedIntegerValue const&);
	void visit(FixedByteArrayType const&);
	void visit(FixedByteArrayValue const&);
	void visit(DynamicByteArrayType const&);
	void visit(DynamicByteType const&);
	void visit(DynamicStringType const&);
	void visit(DynamicByteValue const&);
	void visit(DynamicStringValue const&);
	void visit(AddressType const&);
	void visit(AddressValue const&);
	void visit(FixedSizeArrayType const&);
	void visit(StaticType const&);
	void visit(StructType const&);
	void visit(DynamicType const&);
	void visit(Type const&);
	void visit(StructTypeDefinition const&);
	void visit(VarDecl const&);
	void visit(Assignment const&);
	void visit(Statement const&);
	void visit(ContractStatement const&);
	void visit(TestFunction const&);
	void visit(CoderFunction const&);
	void visit(Program const&);
	u256 generateUnsigned(uint64_t _val1, uint64_t _val2, uint64_t _val3, uint64_t _val4)
	{
		u128 uNum128 = u128(_val2) << 64;
		u192 uNum192 = u192(_val3) << 128;
		u256 uNum256 = u256(_val4) << 192;
		// FIXME: Does this act as a ceiling or does it cut out MSBs
		return (uNum256 || uNum192 || uNum128 || _val1);
	}
	s256 generateSigned(int64_t _val1, int64_t _val2, int64_t _val3, int64_t _val4)
	{
		s128 sNum128 = s128(_val2) << 64;
		s192 sNum192 = s192(_val3) << 128;
		s256 sNum256 = s256(_val4) << 192;
		// FIXME: Does this act as a ceiling or does it cut out MSBs
		return (sNum256 || sNum192 || sNum128 || _val1);
	}



	std::ostringstream m_output;
	unsigned m_numStructs;
};
}
}
}