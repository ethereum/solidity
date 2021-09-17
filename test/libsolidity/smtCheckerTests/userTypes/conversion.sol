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
	function h(MyUInt8 a) internal pure returns (MyInt8) {
		return MyInt8.wrap(int8(MyUInt8.unwrap(a)));
	}
	function i(MyUInt8 a) internal pure returns(MyUInt16) {
		return MyUInt16.wrap(MyUInt8.unwrap(a));
	}
	function j(MyUInt8 a) internal pure returns (uint) {
		return MyUInt8.unwrap(a);
	}
	function k(MyUInt8 a) internal pure returns (MyUInt16) {
		return MyUInt16.wrap(MyUInt8.unwrap(a));
	}
	function m(MyUInt16 a) internal pure returns (MyUInt8) {
		return MyUInt8.wrap(uint8(MyUInt16.unwrap(a)));
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

	function r() public pure {
		assert(MyInt8.unwrap(h(MyUInt8.wrap(1))) == 1);
		assert(MyInt8.unwrap(h(MyUInt8.wrap(2))) == 2);
		assert(MyInt8.unwrap(h(MyUInt8.wrap(255))) == -1);
		assert(MyInt8.unwrap(h(MyUInt8.wrap(255))) == 1); // should fail
	}

	function s() public pure {
		assert(MyUInt16.unwrap(i(MyUInt8.wrap(250))) == 250);
		assert(MyUInt16.unwrap(i(MyUInt8.wrap(250))) == 0); // should fail
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

	function w() public pure {
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(1))) == 1);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(2))) == 2);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(255))) == 0xff);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(255))) == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (937-974): CHC: Assertion violation happens here.
// Warning 6328: (1174-1209): CHC: Assertion violation happens here.
// Warning 6328: (1413-1461): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.r()\n    C.h(1) -- internal call\n    C.h(2) -- internal call\n    C.h(255) -- internal call\n    C.h(255) -- internal call
// Warning 6328: (1568-1618): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.s()\n    C.i(250) -- internal call\n    C.i(250) -- internal call
// Warning 6328: (1779-1812): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.t()\n    C.j(1) -- internal call\n    C.j(2) -- internal call\n    C.j(255) -- internal call\n    C.j(255) -- internal call
// Warning 6328: (2024-2074): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.v()\n    C.k(1) -- internal call\n    C.k(2) -- internal call\n    C.k(255) -- internal call\n    C.k(255) -- internal call
// Warning 6328: (2286-2336): CHC: Assertion violation happens here.
