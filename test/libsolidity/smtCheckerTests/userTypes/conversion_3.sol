pragma abicoder v2;

type MyUInt8 is uint8;
type MyInt8 is int8;
type MyUInt16 is uint16;

contract C {
	function j(MyUInt8 a) internal pure returns (uint) {
		return MyUInt8.unwrap(a);
	}
	function k(MyUInt8 a) internal pure returns (MyUInt16) {
		return MyUInt16.wrap(MyUInt8.unwrap(a));
	}

	function t() public pure {
		assert(j(MyUInt8.wrap(1)) == 1);
		assert(j(MyUInt8.wrap(2)) == 2);
		assert(j(MyUInt8.wrap(255)) == 0xff);
		assert(j(MyUInt8.wrap(255)) == 1); // should fail
	}

	function v() public pure {
		assert(MyUInt16.unwrap(k(MyUInt8.wrap(1))) == 1);
		assert(MyUInt16.unwrap(k(MyUInt8.wrap(2))) == 2);
		assert(MyUInt16.unwrap(k(MyUInt8.wrap(255))) == 0xff);
		assert(MyUInt16.unwrap(k(MyUInt8.wrap(255))) == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (434-467): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.t()\n    C.j(1) -- internal call\n    C.j(2) -- internal call\n    C.j(255) -- internal call\n    C.j(255) -- internal call
// Warning 6328: (679-729): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.v()\n    C.k(1) -- internal call\n    C.k(2) -- internal call\n    C.k(255) -- internal call\n    C.k(255) -- internal call
// Info 1180: Contract invariant(s) for :C:\n(true || true || true || true || true)\nReentrancy property(ies) for :C:\n(((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true || true || true || true || true || true || true)\n(true || true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true || true || true || true)\n(true || true || true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true || true || true || true)\n(true || true || true || true || true || true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))) || true)\n(true || true || true || true || true || true || true || true || ((<errorCode> = 0) && ((:var 0) = (:var 1))))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(j(MyUInt8.wrap(1)) == 1)\n<errorCode> = 2 -> Assertion failed at assert(j(MyUInt8.wrap(2)) == 2)\n<errorCode> = 3 -> Assertion failed at assert(j(MyUInt8.wrap(255)) == 0xff)\n<errorCode> = 4 -> Assertion failed at assert(j(MyUInt8.wrap(255)) == 1)\n<errorCode> = 6 -> Assertion failed at assert(MyUInt16.unwrap(k(MyUInt8.wrap(1))) == 1)\n<errorCode> = 7 -> Assertion failed at assert(MyUInt16.unwrap(k(MyUInt8.wrap(2))) == 2)\n<errorCode> = 8 -> Assertion failed at assert(MyUInt16.unwrap(k(MyUInt8.wrap(255))) == 0xff)\n<errorCode> = 9 -> Assertion failed at assert(MyUInt16.unwrap(k(MyUInt8.wrap(255))) == 1)\n
