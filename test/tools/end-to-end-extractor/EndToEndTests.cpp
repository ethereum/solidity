#include "EndToEndTests.h"

using namespace std;
using namespace std::placeholders;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::test;
using namespace solidity::frontend;

 #define END_TO_END_EXTRACTOR

#define encodeArgs extractor_encodeArgs
#define encode extractor_encodeArgs
#define encodeDyn extractor_encodeDyn
#define asString extractor_asString
#define fromHex extractor_fromHex
#define m_contractAddress extractor_m_contractAddress()
#define m_evmHost extractor_m_evmHost()
#define m_output extractor_m_output()
#define m_compiler extractor_m_compiler()
#define EVMHost FakeEvmHost

#define SolidityExecutionFramework solidity::test::ExtractorExecutionFramework

#define BOOST_FIXTURE_TEST_SUITE(A, B)   \
	End2EndExtractor::End2EndExtractor() \
	{
#define BOOST_AUTO_TEST_SUITE_END() }

#define BOOST_REQUIRE_EQUAL(A, B) \
	if (A == B)                   \
	{                             \
	}

#define BOOST_CHECK_LE(A, B) \
	if (A == B)                   \
	{                             \
	}

#define BOOST_REQUIRE(A) \
	if (A)               \
	{                    \
	}

#define BOOST_CHECK_EQUAL(A, B) \
	if (A == B)                 \
	{                           \
	}
#define BOOST_CHECK(A) \
	if (A)             \
	{                  \
	}

#define ALSO_VIA_YUL(CODE)       \
	{                            \
		m_current->alsoViaYul(); \
		{                        \
			CODE                 \
		}                        \
	};

#define BOOST_AUTO_TEST_CASE(X) m_tests[#X] = ExtractionTask(#X, [this]() { \
	    prepareTest(#X, m_tests);

#ifdef END_TO_END_EXTRACTOR

#include "test/libsolidity/SolidityEndToEndTest.cpp"

#else

BOOST_FIXTURE_TEST_SUITE(SolidityEndToEndTest, SolidityExecutionFramework)

BOOST_AUTO_TEST_CASE(function_types_sig)
{
	char const *sourceCode = R"(
		contract C {
			uint public x;
			function f() public pure returns (bytes4) {
				return this.f.selector;
			}
			function g() public returns (bytes4) {
				function () pure external returns (bytes4) fun = this.f;
				return fun.selector;
			}
			function h() public returns (bytes4) {
				function () pure external returns (bytes4) fun = this.f;
				return fun.selector;
			}
			function i() public pure returns (bytes4) {
				return this.x.selector;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(asString(FixedHash<4>(util::keccak256("f()")).asBytes())));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(asString(FixedHash<4>(util::keccak256("f()")).asBytes())));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(asString(FixedHash<4>(util::keccak256("f()")).asBytes())));
	ABI_CHECK(callContractFunction("i()"), encodeArgs(asString(FixedHash<4>(util::keccak256("x()")).asBytes())));
}
});

BOOST_AUTO_TEST_CASE(constant_string)
{
	char const *sourceCode = R"(
		contract C {
			bytes constant a = "\x03\x01\x02";
			bytes constant b = hex"030102";
			string constant c = "hello";
			function f() public returns (bytes memory) {
				return a;
			}
			function g() public returns (bytes memory) {
				return b;
			}
			function h() public returns (bytes memory) {
				return bytes(c);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeDyn(string("\x03\x01\x02")));
	ABI_CHECK(callContractFunction("g()"), encodeDyn(string("\x03\x01\x02")));
	ABI_CHECK(callContractFunction("g()"), encodeDyn(3, 1, 2));
	ABI_CHECK(callContractFunction("h()"), encodeDyn(string("hello")));
}
});

BOOST_AUTO_TEST_CASE(address_overload_resolution)
{
	char const *sourceCode = R"(
		contract C {
			function balance() public returns (uint) {
				return 1;
			}
			function transfer(uint amount) public returns (uint) {
				return amount;
			}
		}
		contract D {
			function f() public returns (uint) {
				return (new C()).balance();
			}
			function g() public returns (uint) {
				return (new C()).transfer(5);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	BOOST_CHECK(callContractFunction("f()") == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("g()") == encodeArgs(u256(5)));
}
});

BOOST_AUTO_TEST_CASE(do_while_loop)
{
	char const *sourceCode = R"(
		contract test {
			function f(uint n) public returns(uint nfac) {
				nfac = 1;
				uint i = 2;
				do { nfac *= i++; } while (i <= n);
			}
		}
	)";
	ALSO_VIA_YUL(compileAndRun(sourceCode);

	             auto do_while_loop_cpp = [](u256 const &n) -> u256 {
		             u256 nfac = 1;
		             u256 i = 2;
		             do
		             {
			             nfac *= i++;
		             } while (i <= n);

		             return nfac;
	             };

	             testContractAgainstCppOnRange("f(uint256)", do_while_loop_cpp, 0, 5);)
}
});

BOOST_AUTO_TEST_CASE(constructor_with_long_arguments)
{
	char const *sourceCode = R"(
		contract Main {
			string public a;
			string public b;

			constructor(string memory _a, string memory _b) public {
				a = _a;
				b = _b;
			}
		}
	)";
	string a
	    = "01234567890123gabddunaouhdaoneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi4567890789012oneuda"
	      "pcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi45"
	      "678907890123456789abcd123456787890123456789abcd90123456789012345678901234567890123456789aboneudapcgadi456789"
	      "0789012oneudapcgadi4567890789012oneudapcgadi45678907890123456789abcd123456787890123456789abcd901234567890123"
	      "45678901234567890123456789aboneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi456789078901234567"
	      "89abcd123456787890123456789abcd90123456789012345678901234567890123456789aboneudapcgadi4567890789012cdef";
	string b = "AUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>"
	           "PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PY";

	compileAndRun(sourceCode,
	              0,
	              "Main",
	              encodeArgs(u256(0x40),
	                         u256(0x40 + 0x20 + ((a.length() + 31) / 32) * 32),
	                         u256(a.length()),
	                         a,
	                         u256(b.length()),
	                         b));
	ABI_CHECK(callContractFunction("a()"), encodeDyn(a));
	ABI_CHECK(callContractFunction("b()"), encodeDyn(b));
}
});

BOOST_AUTO_TEST_CASE(constructor_static_array_argument)
{
	char const *sourceCode = R"(
		contract C {
			uint public a;
			uint[3] public b;

			constructor(uint _a, uint[3] memory _b) public {
				a = _a;
				b = _b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C", encodeArgs(u256(1), u256(2), u256(3), u256(4)));
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("b(uint256)", 0, string("iaaaz32"), "3"), encodeArgs(u256(2), 4, "111"));
	ABI_CHECK(callContractFunction("b(uint256)", u256(1)), encodeArgs(u256(3)));
	ABI_CHECK(callContractFunction("b(uint256)", u256(2)), encodeArgs(u256(4)));
}
});

BOOST_AUTO_TEST_CASE(constant_var_as_array_length)
{
	char const *sourceCode = R"(
		contract C {
			uint constant LEN = 3;
			uint[LEN] public a;

			constructor(uint[LEN] memory _a) public {
				a = _a;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C", encodeArgs(u256(1), u256(2), u256(3)));
	ABI_CHECK(callContractFunction("a(uint256)", u256(0)), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("a(uint256)", u256(1)), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("a(uint256)", u256(2)), encodeArgs(u256(3)));
}
});

BOOST_AUTO_TEST_CASE(struct_referencing)
{
	static char const *sourceCode = R"(
		pragma experimental ABIEncoderV2;
		interface I {
			struct S { uint a; }
		}
		library L {
			struct S { uint b; uint a; }
			function f() public pure returns (S memory) {
				S memory s;
				s.a = 3;
				return s;
			}
			function g() public pure returns (I.S memory) {
				I.S memory s;
				s.a = 4;
				return s;
			}
			// argument-dependant lookup tests
			function a(I.S memory) public pure returns (uint) { return 1; }
			function a(S memory) public pure returns (uint) { return 2; }
		}
		contract C is I {
			function f() public pure returns (S memory) {
				S memory s;
				s.a = 1;
				return s;
			}
			function g() public pure returns (I.S memory) {
				I.S memory s;
				s.a = 2;
				return s;
			}
			function h() public pure returns (L.S memory) {
				L.S memory s;
				s.a = 5;
				return s;
			}
			function x() public pure returns (L.S memory) {
				return L.f();
			}
			function y() public pure returns (I.S memory) {
				return L.g();
			}
			function a1() public pure returns (uint) { S memory s; return L.a(s); }
			function a2() public pure returns (uint) { L.S memory s; return L.a(s); }
		}
	)";
	compileAndRun(sourceCode, 0, "L");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0, 3));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(4, -1, u256(-1)));
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"L", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(2));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(0, 5));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(0, 3));
	ABI_CHECK(callContractFunction("y()"), encodeArgs(4));
	ABI_CHECK(callContractFunction("a1()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("a2()"), encodeArgs(2));
}
});

BOOST_AUTO_TEST_CASE(fixed_arrays_in_storage)
{
	char const *sourceCode = R"(
		contract c {
			struct Data { uint x; uint y; }
			Data[2**10] data;
			uint[2**10 + 3] ids;
			function setIDStatic(uint id) public { ids[2] = id; }
			function setID(uint index, uint id) public { ids[index] = id; }
			function setData(uint index, uint x, uint y) public { data[index].x = x; data[index].y = y; }
			function getID(uint index) public returns (uint) { return ids[index]; }
			function getData(uint index) public returns (uint x, uint y) { x = data[index].x; y = data[index].y; }
			function getLengths() public returns (uint l1, uint l2) { l1 = data.length; l2 = ids.length; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("setIDStatic(uint256)", 11), bytes());
	ABI_CHECK(callContractFunction("getID(uint256)", 2), encodeArgs(11));
	ABI_CHECK(callContractFunction("setID(uint256,uint256)", 7, 8), bytes());
	ABI_CHECK(callContractFunction("getID(uint256)", 7), encodeArgs(8));
	ABI_CHECK(callContractFunction("setData(uint256,uint256,uint256)", 7, 8, 9), bytes());
	ABI_CHECK(callContractFunction("setData(uint256,uint256,uint256)", 8, 10, 11), bytes());
	ABI_CHECK(callContractFunction("getData(uint256)", 7), encodeArgs(8, 9));
	ABI_CHECK(callContractFunction("getData(uint256)", 8), encodeArgs(10, 11));
	ABI_CHECK(callContractFunction("getLengths()"), encodeArgs(u256(1) << 10, (u256(1) << 10) + 3));
}
});

BOOST_AUTO_TEST_SUITE_END()

#endif
