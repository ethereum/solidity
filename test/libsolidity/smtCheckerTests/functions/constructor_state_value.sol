pragma experimental SMTChecker;

contract C {
	uint x = 5;

	constructor() public {
		assert(x == 5);
		x = 10;
	}

	function f(uint y) public view {
		assert(y == x);
	}
}
// ----
// Warning: (152-166): Assertion violation happens here
