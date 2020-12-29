pragma experimental SMTChecker;

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
// ----
// Warning 6328: (187-201): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf()
// Warning 6328: (385-399): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\nd = 0\n\n\nTransaction trace:\nconstructor()\nState: x = 1\ncall(0)
