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
// Warning 4661: (145-159): Assertion violation happens here
// Warning 2661: (163-166): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 4661: (227-241): Assertion violation happens here
// Warning 4661: (252-266): Assertion violation happens here
// Warning 4661: (177-191): Assertion violation happens here
// Warning 4661: (227-241): Assertion violation happens here
// Warning 4144: (245-248): Underflow (resulting value less than 0) happens here
// Warning 4661: (252-266): Assertion violation happens here
// Warning 4661: (89-103): Assertion violation happens here
