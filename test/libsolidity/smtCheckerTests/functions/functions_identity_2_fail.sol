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
// Warning 6328: (229-242): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\ng()
