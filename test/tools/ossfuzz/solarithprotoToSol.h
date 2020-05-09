#include <test/tools/ossfuzz/solArith.pb.h>

#include "yulFuzzerCommon.h"
#include <liblangutil/EVMVersion.h>
#include <libsolutil/Common.h>
#include <libsolutil/FixedHash.h>
#include <libsolutil/Keccak256.h>
#include <random>

namespace solidity::test::solarithfuzzer
{
/// Random number generator that is seeded with a fuzzer
/// supplied unsigned integer.
struct SolRandomNumGenerator
{
	using RandomEngine = std::mt19937_64;

	explicit SolRandomNumGenerator(unsigned _seed): m_random(RandomEngine(_seed)) {}

	/// @returns a pseudo random unsigned integer
	unsigned operator()()
	{
		return m_random();
	}

	RandomEngine m_random;
};

enum class Sign
{
	Signed,
	Unsigned
};

class ProtoConverter
{
public:
	ProtoConverter()
	{

	}
	ProtoConverter(ProtoConverter const&) = delete;
	ProtoConverter(ProtoConverter&&) = delete;
	std::string programToString(Program const& _input);
private:
	std::string visit(Type const& _type);
	std::string visit(Assembly const& _a);
	std::string visit(VarDecl const& _decl);
	std::string visit(Block const& _block);
	std::string visit(Program const& _program);

	std::map<u256, std::string> interpretYul(
		std::string const& _yulSource,
		langutil::EVMVersion _version,
		std::ostringstream& _os
	);
	std::string addChecks(
		std::string const& _yulSource,
		langutil::EVMVersion _version,
		std::ostringstream& _os
	);

	std::string newVarName()
	{
		return "v" + std::to_string(m_varCounter);
	}
	void incrementVarCounter()
	{
		m_varCounter++;
	}
	static std::string signString(Type::Sign _sign)
	{
		return _sign == Type::Sign::Type_Sign_SIGNED ? "int" : "uint";
	}
	static std::string signString(Sign _sign)
	{
		return _sign == Sign::Signed ? "int" : "uint";
	}
	static std::string widthString(unsigned _width)
	{
		return std::to_string((_width % 32 + 1) * 8);
	}
	static unsigned widthUnsigned(unsigned _width)
	{
		return _width % 32 + 1;
	}
	bool varAvailable()
	{
		return m_varCounter > 0;
	}
	Sign typeSign(Type const& _ts)
	{
		return _ts.s() == Type::SIGNED ? Sign::Signed : Sign::Unsigned;
	}
	std::string maskUnsignedToHex(unsigned _numMaskNibbles)
	{
		return toHex(maskUnsignedInt(_numMaskNibbles), util::HexPrefix::Add);
	}

	// Convert _counter to string and return its keccak256 hash
	solidity::u256 hashUnsignedInt()
	{
		return util::keccak256(util::h256((*m_rand)()));
	}
	u256 maskUnsignedInt(unsigned _numMaskNibbles)
	{
		return hashUnsignedInt() & u256("0x" + std::string(_numMaskNibbles, 'f'));
	}
	static std::string extractBytes(std::string _value, unsigned _numBytes)
	{
		return _value.substr(_value.size() - (_numBytes * 2));
	}

	unsigned m_varCounter = 0;
	std::shared_ptr<SolRandomNumGenerator> m_rand;
	std::map<std::string, std::tuple<Sign, std::string, unsigned>> m_varTypeMap;
	std::string m_yulAssembly;
	std::string m_yulProgram;
	std::ostringstream m_yulInitCode;
};
}
