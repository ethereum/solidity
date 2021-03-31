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

// ====
// SMTEngine: all
// ----
// Warning 6328: (197-210): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g()\n    C.h(0) -- internal call\n        C.k(0) -- internal call
