contract C{
    uint x;
	constructor(uint y) {
		assert(x == 1);
		x = 1;
	}
    function f() public {
		assert(x == 2);
		++x;
		++x;
		g();
		g();
		assert(x == 3);
    }

	function g() internal {
		--x;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 5667: (37-43): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (49-63): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\ny = 0\n\nTransaction trace:\nC.constructor(0)
// Warning 6328: (105-119): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\nTransaction trace:\nC.constructor(0)\nState: x = 1\nC.f()
// Warning 6328: (151-165): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\nTransaction trace:\nC.constructor(0)\nState: x = 1\nC.f()\n    C.g() -- internal call\n    C.g() -- internal call
