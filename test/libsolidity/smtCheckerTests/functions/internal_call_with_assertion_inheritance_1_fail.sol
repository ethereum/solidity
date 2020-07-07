pragma experimental SMTChecker;

contract A {
	uint x;
	function f() internal {
		assert(x == 2);
		--x;
	}
}

contract C is A {
	constructor() {
		assert(x == 1);
		++x;
		f();
		assert(x == 1);
	}
}
// ----
// Warning 4661: (82-96): Assertion violation happens here
// Warning 4144: (100-103): Underflow (resulting value less than 0) happens here
// Warning 4661: (82-96): Assertion violation happens here
// Warning 4144: (100-103): Underflow (resulting value less than 0) happens here
// Warning 4661: (148-162): Assertion violation happens here
// Warning 4661: (82-96): Assertion violation happens here
// Warning 4661: (180-194): Assertion violation happens here
