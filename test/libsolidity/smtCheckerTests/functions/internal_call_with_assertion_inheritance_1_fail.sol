pragma experimental SMTChecker;

contract A {
	uint x;
	function f() internal {
		assert(x == 2);
		--x;
	}
}

contract C is A {
	constructor() public {
		assert(x == 1);
		++x;
		f();
		assert(x == 1);
	}
}
// ----
// Warning: (82-96): Assertion violation happens here
// Warning: (100-103): Underflow (resulting value less than 0) happens here
// Warning: (82-96): Assertion violation happens here
// Warning: (100-103): Underflow (resulting value less than 0) happens here
// Warning: (155-169): Assertion violation happens here
// Warning: (82-96): Assertion violation happens here
// Warning: (187-201): Assertion violation happens here
