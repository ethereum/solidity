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
		++x;
		g();
		g();
		assert(x == 3);
    }

	function g() internal {
		--x;
	}
}
// ----
// Warning 5667: (70-76): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (138-152): Assertion violation happens here
// Warning 6328: (184-198): Assertion violation happens here
// Warning 6328: (82-96): Assertion violation happens here
// Warning 2661: (156-159): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (163-166): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 2661: (234-237): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 4144: (234-237): Underflow (resulting value less than 0) happens here
