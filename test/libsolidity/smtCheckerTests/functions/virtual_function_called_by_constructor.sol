pragma experimental SMTChecker;
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
// ----
// Warning 6328: (231-246): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nA.constructor()\nState: x = 2\nA.i()
// Warning 6328: (419-433): CHC: Assertion violation happens here.\nCounterexample:\nx = 10\n\nTransaction trace:\nC.constructor()\nState: x = 10\nC.i()
