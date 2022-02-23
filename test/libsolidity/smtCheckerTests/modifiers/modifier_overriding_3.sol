abstract contract A {
	bool s;

	function f() public view mod {
		assert(s); // holds for B
		assert(!s); // fails for B
	}
	modifier mod() virtual;
}

contract B is A {
	modifier mod() virtual override {
		bool x = true;
		s = x;
		_;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (94-104): CHC: Assertion violation happens here.\nCounterexample:\ns = true\nx = true\n\nTransaction trace:\nB.constructor()\nState: s = false\nA.f()
