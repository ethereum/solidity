pragma experimental SMTChecker;

contract C {
	uint x = 5;

	constructor(uint a, uint b) public {
		assert(x == 5);
		x = a + b;
	}

	function f(uint y) public view {
		assert(y == x);
	}
}
// ----
// Warning: (169-183): Assertion violation happens here
// Warning: (122-127): Overflow (resulting value larger than 2**256 - 1) happens here
