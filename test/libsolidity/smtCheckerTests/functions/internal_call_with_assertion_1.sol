pragma experimental SMTChecker;

contract C{
    uint x;
	constructor(uint y) public {
		assert(x == 0);
		x = 1;
	}
    function f() public {
		assert(x == 1);
		++x;
		g();
		assert(x == 1);
    }

	function g() internal {
		assert(x == 2);
		--x;
		assert(x == 1);
	}
}
// ----
// Warning: (70-76): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (163-166): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (245-248): Underflow (resulting value less than 0) happens here
