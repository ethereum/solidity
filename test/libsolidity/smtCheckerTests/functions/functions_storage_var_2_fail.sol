pragma experimental SMTChecker;
contract C
{
	uint a;
	function f(uint x) public {
		uint y;
		a = (y = x);
	}
	function g() public {
		f(1);
		f(0);
		assert(a > 0);
	}
}

// ----
// Warning: (152-165): Assertion violation happens here
