pragma experimental SMTChecker;
contract C
{
	function h(uint x) public pure returns (uint) {
		return k(x);
	}

	function k(uint x) public pure returns (uint) {
		return x;
	}
	function g() public pure {
		uint x;
		x = h(0);
		assert(x > 0);
	}
}

// ----
// Warning: (229-242): Assertion violation happens here
