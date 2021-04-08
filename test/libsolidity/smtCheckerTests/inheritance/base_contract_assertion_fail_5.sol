contract A {
	uint x;
	function f() internal virtual {
		v();
		assert(x == 0); // should hold
		assert(x == 2); // should fail
	}
	function v() internal virtual {
		x = 0;
	}
}

contract B is A {
	function f() internal virtual override {
		super.f();
	}
}

contract C is B {
	function g() public {
		x = 1;
		f();
	}
	function v() internal override {
		x = 2;
		super.v();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (97-111): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.g()\n    B.f() -- internal call\n        A.f() -- internal call\n            C.v() -- internal call\n                A.v() -- internal call
