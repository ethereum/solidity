#include <test/tools/ossfuzz/solArith.pb.h>

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

struct SolVarRef
{
	SolVarRef(VarRef const& _varref);

	std::string str();

	Sign m_sign;
};

struct SolExpression
{
	SolExpression(Expression const& _e);

	std::string str();

	Sign m_sign;
};

struct SolBinop
{
	SolBinop(SolExpression const& _left, SolExpression const& _right);

	std::string str();

	Sign m_sign;
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
	std::string visit(UnaryOpStmt const& _uop);
	std::string visit(BinaryOp const& _bop);
	std::string visit(VarDecl const& _decl);
	std::string visit(Assignment const& _assignment);
	std::string visit(Return const& _return);
	std::string visit(VarRef const& _varref);
	std::string visit(Expression const& _expr);
	std::string visit(Statement const& _stmt);
	std::string visit(Block const& _block);
	std::string visit(Program const& _program);

	std::string newVarName()
	{
		return "v" + std::to_string(m_varCounter);
	}
	void incrementVarCounter()
	{
		m_varCounter++;
	}
	std::string randomVarName()
	{
		return "v" + std::to_string((*m_rand)() % m_varCounter);
	}
	static std::string signString(Type::Sign _sign)
	{
		return _sign == Type::Sign::Type_Sign_SIGNED ? "uint" : "uint";
	}
	static std::string widthString(unsigned _width)
	{
		return std::to_string((_width % 32 + 1) * 8);
	}
	bool varAvailable()
	{
		return m_varCounter > 0;
	}
	Sign typeSign(Type const& _ts)
	{
		return _ts.s() == Type::SIGNED ? Sign::Signed : Sign::Unsigned;
	}

	unsigned m_varCounter = 0;
	std::shared_ptr<SolRandomNumGenerator> m_rand;
	std::map<std::string, std::pair<Sign, std::string>> m_varTypeMap;
	std::map<Expression const*, Sign> m_exprSignMap;
	std::string m_returnType;
};
}