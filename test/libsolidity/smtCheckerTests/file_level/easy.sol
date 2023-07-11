function add(uint a, uint b) pure returns (uint) {
	return a + b;
}

contract C {
	function f(uint x) internal pure returns (uint) {
		return add(x, 2);
	}
	function g() public pure {
		assert(f(7) == 9); // should hold
		assert(f(8) == 9); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (222-239): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
