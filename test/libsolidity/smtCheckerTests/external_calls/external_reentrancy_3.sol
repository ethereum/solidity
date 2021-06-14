abstract contract D {
	function d() virtual public {}
}

contract A {
	int x = 0;

	function f() virtual public view {
		assert(x == 0); // should hold
		assert(x == 1); // should fail
	}
}
contract C is A {
	constructor() {
		x = 1;
	}

	function call(D d) public {
		d.d();
	}

	function f() public view override {
		assert(x == 1); // should hold
		assert(x == 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (154-168): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.f()
// Warning 6328: (352-366): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\nd = 0\n\nTransaction trace:\nC.constructor()\nState: x = 1\nC.call(0)\n    d.d() -- untrusted external call, synthesized as:\n        C.f() -- reentrant call
// Warning 0: (190-387): Contract invariants for :C:\n(!(x <= 0) && !(x >= 2))\n(!(x >= 2) && !(x <= 0))\n((!(x <= 1) || !(x' >= 2)) && (!(x' <= 0) || (x <= 0)))\n((!(x' <= 0) || (x <= 0)) && (!(<errorCode> = 5) || !(x = 1)) && (!(x <= 1) || !(x' >= 2)))\n
