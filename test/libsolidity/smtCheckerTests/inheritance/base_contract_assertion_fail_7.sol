contract A {
	uint x;
	function f() internal {
		v();
		assert(x == 0); // should fail
		assert(x == 2); // should hold
	}
	function v() internal virtual {
		x = 0;
	}
}

contract C is A {
	function g() public {
		x = 1;
		f();
	}
	function v() internal override {
		x = 2;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (56-70): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.g()\n    A.f() -- internal call\n        C.v() -- internal call
