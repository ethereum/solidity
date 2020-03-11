pragma experimental SMTChecker;

contract C{
    uint x;
	constructor(uint y) public {
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
// Warning: (70-76): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (145-159): Assertion violation happens here
// Warning: (163-166): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (170-173): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (241-244): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (191-205): Assertion violation happens here
// Warning: (241-244): Underflow (resulting value less than 0) happens here
// Warning: (89-103): Assertion violation happens here
