pragma experimental SMTChecker;

contract C{
    uint x;
	constructor(uint y) {
		assert(x == 1);
		x = 1;
	}
    function f() public {
		assert(x == 2);
		++x;
		g();
		assert(x == 2);
    }

	function g() internal {
		assert(x == 3);
		--x;
		assert(x == 2);
	}
}
// ----
// Warning 5667: (70-76): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (138-152): Assertion violation happens here
// Warning 6328: (170-184): Assertion violation happens here
// Warning 6328: (220-234): Assertion violation happens here
// Warning 6328: (245-259): Assertion violation happens here
// Warning 6328: (82-96): Assertion violation happens here
// Warning 2661: (156-159): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 4144: (238-241): Underflow (resulting value less than 0) happens here
