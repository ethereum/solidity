contract A {
	uint x;
	function f() internal virtual {
		v();
		assert(x == 2); // should hold
	}
	function v() internal virtual {
		x = 0;
	}
	function g() public virtual {
		v();
		assert(x == 2); // should fail
	}
}

contract B is A {
	function f() internal virtual override {
		super.f();
	}
}

contract C is B {
	function g() public override {
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
// Warning 6328: (183-197): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.g()\n    A.v() -- internal call
