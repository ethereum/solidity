contract C {
	type T is uint;
}
contract D {
	function f(C.T x) internal pure returns(uint) {
		return C.T.unwrap(x);
	}
	function g(uint x) internal pure returns(C.T) {
		return C.T.wrap(x);
	}
	function h(uint x) internal pure returns(uint) {
		return f(g(x));
	}
	function i(C.T x) internal pure returns(C.T) {
		return g(f(x));
	}

	function m() public pure {
		assert(f(C.T.wrap(0x42)) == 0x42);
		assert(f(C.T.wrap(0x42)) == 0x43); // should fail
		assert(C.T.unwrap(g(0x42)) == 0x42);
		assert(C.T.unwrap(g(0x42)) == 0x43); // should fail
		assert(h(0x42) == 0x42);
		assert(h(0x42) == 0x43); // should fail
		assert(C.T.unwrap(i(C.T.wrap(0x42))) == 0x42);
		assert(C.T.unwrap(i(C.T.wrap(0x42))) == 0x43); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (403-436): CHC: Assertion violation happens here.
// Warning 6328: (494-529): CHC: Assertion violation happens here.
// Warning 6328: (575-598): CHC: Assertion violation happens here.
// Warning 6328: (666-711): CHC: Assertion violation happens here.
