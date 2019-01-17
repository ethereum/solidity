pragma experimental SMTChecker;
contract C
{
	function h(uint x) public pure returns (uint) {
		return x;
	}
	function g() public pure {
		uint x;
		x = (h)(0);
		assert(x > 0);
	}
}

// ----
// Warning: (153-156): Assertion checker does not yet implement tuples and inline arrays.
// Warning: (163-176): Assertion violation happens here
