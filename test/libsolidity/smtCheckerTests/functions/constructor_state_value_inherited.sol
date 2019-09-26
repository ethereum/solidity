pragma experimental SMTChecker;

contract B {
	uint x = 5;
}

contract C is B {
	constructor() public {
		assert(x == 5);
		x = 10;
	}

	function f(uint y) public view {
		assert(y == x);
	}
}
// ----
// Warning: (172-186): Assertion violation happens here
