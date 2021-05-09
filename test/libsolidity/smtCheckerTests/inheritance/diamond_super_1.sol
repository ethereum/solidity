contract A {
	function f() public virtual returns (uint256 r) {
		return 1;
	}
}


contract B is A {
	function f() public virtual override returns (uint256 r) {
		return super.f() + 2;
	}
}


contract C is A {
	function f() public virtual override returns (uint256 r) {
		return super.f() + 4;
	}
}


contract D is B, C {
	function f() public override(B, C) returns (uint256 r) {
		r = super.f() + 8;
		assert(r == 15); // should hold
		assert(r == 13); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (437-452): CHC: Assertion violation happens here.\nCounterexample:\n\nr = 15\n\nTransaction trace:\nD.constructor()\nD.f()\n    C.f() -- internal call\n        B.f() -- internal call\n            A.f() -- internal call
