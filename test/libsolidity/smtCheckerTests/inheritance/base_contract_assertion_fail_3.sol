contract A {
	uint x;
	function f() internal virtual {
		v();
		assert(x == 0); // should fail
		assert(x == 2); // should hold
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
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 6328: (64-78): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.g()\n    B.f() -- internal call\n        A.f() -- internal call\n            C.v() -- internal call
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
