pragma abicoder v2;

type MyUInt8 is uint8;
type MyInt8 is int8;
type MyUInt16 is uint16;

contract C {
	function m(MyUInt16 a) internal pure returns (MyUInt8) {
		return MyUInt8.wrap(uint8(MyUInt16.unwrap(a)));
	}

	function w() public pure {
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(1))) == 1);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(2))) == 2);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(255))) == 0xff);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(255))) == 1); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (407-457): CHC: Assertion violation happens here.
// Info 1180: Contract invariant(s) for :C:\n(true || true || true)\nReentrancy property(ies) for :C:\n(((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true || true || true)\n(true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(MyUInt8.unwrap(m(MyUInt16.wrap(1))) == 1)\n<errorCode> = 2 -> Assertion failed at assert(MyUInt8.unwrap(m(MyUInt16.wrap(2))) == 2)\n<errorCode> = 3 -> Assertion failed at assert(MyUInt8.unwrap(m(MyUInt16.wrap(255))) == 0xff)\n<errorCode> = 4 -> Assertion failed at assert(MyUInt8.unwrap(m(MyUInt16.wrap(255))) == 1)\n
