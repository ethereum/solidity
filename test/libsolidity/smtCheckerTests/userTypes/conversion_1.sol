pragma abicoder v2;

type MyUInt8 is uint8;
type MyInt8 is int8;
type MyUInt16 is uint16;

contract C {
	function f(uint a) internal pure returns(MyUInt8) {
		return MyUInt8.wrap(uint8(a));
	}
	function g(uint a) internal pure returns(MyInt8) {
		return MyInt8.wrap(int8(int(a)));
	}

	function p() public pure {
		assert(MyUInt8.unwrap(f(1)) == 1);
		assert(MyUInt8.unwrap(f(2)) == 2);
		assert(MyUInt8.unwrap(f(257)) == 1);
		assert(MyUInt8.unwrap(f(257)) == 257); // should fail
	}

	function q() public pure {
		assert(MyInt8.unwrap(g(1)) == 1);
		assert(MyInt8.unwrap(g(2)) == 2);
		assert(MyInt8.unwrap(g(255)) == -1);
		assert(MyInt8.unwrap(g(257)) == 1);
		assert(MyInt8.unwrap(g(257)) == -1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (428-465): CHC: Assertion violation happens here.
// Warning 6328: (665-700): CHC: Assertion violation happens here.
// Info 1180: Contract invariant(s) for :C:\n(true || true || true || true || true)\nReentrancy property(ies) for :C:\n(true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true || true || true || true || true || true)\n(true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true || true || true || true || true)\n(true || true || true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true || true || true)\n(true || true || true || true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true || true)\n(true || true || true || true || true || true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(MyUInt8.unwrap(f(1)) == 1)\n<errorCode> = 2 -> Assertion failed at assert(MyUInt8.unwrap(f(2)) == 2)\n<errorCode> = 3 -> Assertion failed at assert(MyUInt8.unwrap(f(257)) == 1)\n<errorCode> = 4 -> Assertion failed at assert(MyUInt8.unwrap(f(257)) == 257)\n<errorCode> = 6 -> Assertion failed at assert(MyInt8.unwrap(g(1)) == 1)\n<errorCode> = 7 -> Assertion failed at assert(MyInt8.unwrap(g(2)) == 2)\n<errorCode> = 8 -> Assertion failed at assert(MyInt8.unwrap(g(255)) == -1)\n<errorCode> = 9 -> Assertion failed at assert(MyInt8.unwrap(g(257)) == 1)\n<errorCode> = 10 -> Assertion failed at assert(MyInt8.unwrap(g(257)) == -1)\n
