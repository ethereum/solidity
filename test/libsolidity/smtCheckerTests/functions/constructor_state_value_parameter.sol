pragma experimental SMTChecker;

contract C {
	uint x = 5;

	constructor(uint a, uint b) {
		assert(x == 5);
		x = a + b;
	}

	function f(uint y) view public {
		assert(y == x);
	}
}
// ----
// Warning 6328: (162-176): Assertion violation happens here
// Warning 2661: (115-120): Overflow (resulting value larger than 2**256 - 1) happens here
