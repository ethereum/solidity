pragma experimental SMTChecker;
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
		return 2 * (super.f() + 4);
	}
}


contract D is B, C {
	function f() public override(B, C) returns (uint256 r) {
		r = super.f() + 8;
		assert(r == 22); // should hold
		assert(r == 20); // should fail
		assert(r == 18); // should fail
	}
}
// ----
// Warning 6328: (475-490): CHC: Assertion violation happens here.\nCounterexample:\n\nr = 22\n\nTransaction trace:\nD.constructor()\nD.f()\n    C.f() -- internal call\n        B.f() -- internal call\n            A.f() -- internal call
// Warning 6328: (509-524): CHC: Assertion violation happens here.\nCounterexample:\n\nr = 22\n\nTransaction trace:\nD.constructor()\nD.f()\n    C.f() -- internal call\n        B.f() -- internal call\n            A.f() -- internal call
