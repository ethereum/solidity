pragma experimental SMTChecker;

contract C{
    uint x;
	constructor(uint y) {
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
// Warning 5667: (70-76): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 2661: (156-159): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 4144: (238-241): Underflow (resulting value less than 0) happens here
