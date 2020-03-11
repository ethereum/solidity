pragma experimental SMTChecker;

contract A {
	uint x;
	function f() internal {
		assert(x == 1);
		--x;
	}
}

contract C is A {
	constructor() public {
		assert(x == 0);
		++x;
		f();
		assert(x == 0);
	}
}
// ----
// Warning: (100-103): Underflow (resulting value less than 0) happens here
// Warning: (100-103): Underflow (resulting value less than 0) happens here
