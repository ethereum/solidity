pragma experimental SMTChecker;

contract C {
	uint x;

	constructor() public {
		assert(x == 0);
		x = 10;
	}

	function f(uint y) public view {
		assert(y == x);
	}
}
// ----
// Warning: (148-162): Assertion violation happens here
