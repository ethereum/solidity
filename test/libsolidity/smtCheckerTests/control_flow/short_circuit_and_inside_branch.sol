contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g() public returns (bool) {
		bool b;
			x = 100;
			b = f() > 0;
			assert(x == 102);
			// Should fail.
			assert(!b);
		return b;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (169-185): CHC: Assertion violation happens here.\nCounterexample:\nx = 101\n = false\nb = true\n\nTransaction trace:\nc.constructor()\nState: x = 0\nc.g()\n    c.f() -- internal call
// Warning 6328: (209-219): CHC: Assertion violation happens here.\nCounterexample:\nx = 101\n = false\nb = true\n\nTransaction trace:\nc.constructor()\nState: x = 0\nc.g()\n    c.f() -- internal call
