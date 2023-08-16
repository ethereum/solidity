contract A {
	int x = 0;

	function f() virtual internal {
		x = 2;
	}

	function proxy() public {
		f();
	}
}

contract C is A {
	function f() internal virtual override {
		super.f();
		assert(x == 2);
		assert(x == 3); // should fail
	}
}

contract D is C {

	function f() internal override {
		super.f();
		assert(x == 2);
		assert(x == 3); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 6328: (205-219): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nC.constructor()\nState: x = 0\nA.proxy()\n    C.f() -- internal call\n        A.f() -- internal call
// Warning 6328: (328-342): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nD.constructor()\nState: x = 0\nA.proxy()\n    D.f() -- internal call\n        C.f() -- internal call\n            A.f() -- internal call
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
