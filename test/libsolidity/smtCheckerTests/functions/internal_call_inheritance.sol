contract C {
	function c() public pure returns (uint) { return 42; }
}

contract B is C {
	function b() public pure returns (uint) { return c(); }
}

contract A is B {
	uint public x;

	function a() public {
		x = b();
		assert(x < 40);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (221-235): CHC: Assertion violation happens here.\nCounterexample:\nx = 42\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.a()\n    B.b() -- internal call\n        C.c() -- internal call
