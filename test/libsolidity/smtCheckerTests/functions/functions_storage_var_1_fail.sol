contract C
{
	uint a;
	function f(uint x) public {
		uint y;
		a = (y = x);
	}
	function g() public {
		f(0);
		assert(a > 0);
	}
}

// ====
// SMTEngine: all
// ----
// Warning 6328: (112-125): CHC: Assertion violation happens here.\nCounterexample:\na = 0\n\nTransaction trace:\nC.constructor()\nState: a = 0\nC.g()\n    C.f(0) -- internal call
