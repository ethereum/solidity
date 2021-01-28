pragma experimental SMTChecker;

contract A {
	uint x;
	function f() public virtual {
		v();
		assert(x == 0); // should fail when C is the most derived contract
		assert(x == 2); // should fail when A is the most derived contract
	}
	function v() internal virtual {
		x = 0;
	}
}

contract B is A {
	function f() public virtual override {
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
// ----
// Warning 6328: (164-178): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.f()\n    A.v() -- internal call
// Warning 6328: (95-109): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nC.constructor()\nState: x = 0\nB.f()\n    A.f() -- internal call\n        C.v() -- internal call
