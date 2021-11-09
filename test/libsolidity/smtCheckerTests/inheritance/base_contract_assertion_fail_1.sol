contract B {
	uint x;
	function f() public view {
		assert(x == 0);
	}
}

contract C is B {
	uint y;
	function g() public {
		x = 1;
		f();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (52-66): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 1\n\nTransaction trace:\nC.constructor()\nState: y = 0, x = 0\nC.g()\n    B.f() -- internal call
// Info 1180: Contract invariant(s) for :B:\n(x <= 0)\n
