contract D {
	uint public d;
	function g() public {
		++d;
	}
}

contract C {
	function f() public {
		D a = new D();
		assert(a.d() == 0); // should hold
		a.g();
		assert(a.d() == 1); // should hold
		assert(a.d() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (203-221): CHC: Assertion violation happens here.\nCounterexample:\n\na = 42\n\nTransaction trace:\nC.constructor()\nC.f()\n    D.g() -- trusted external call
