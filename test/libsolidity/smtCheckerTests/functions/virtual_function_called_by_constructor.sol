contract A {
    uint public x;
    function v() internal virtual {
        x = 2;
    }
    constructor() {
        v();
    }
	function i() public view virtual {
		assert(x == 2); // should hold
		assert(x == 10); // should fail
	}
}

contract C is A {
    function v() internal override {
        x = 10;
    }
	function i() public view override {
		assert(x == 10); // should hold
		assert(x == 2); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (199-214): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nA.constructor()\nState: x = 2\nA.i()
// Warning 6328: (387-401): CHC: Assertion violation happens here.\nCounterexample:\nx = 10\n\nTransaction trace:\nC.constructor()\nState: x = 10\nC.i()
// Info 1180: Contract invariant(s) for :A:\n(!(x <= 1) && !(x >= 3))\nContract invariant(s) for :C:\n(!(x <= 9) && !(x >= 11))\n
