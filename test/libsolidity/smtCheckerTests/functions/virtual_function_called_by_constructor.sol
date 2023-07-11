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
// Warning 6328: (199-214): CHC: Assertion violation happens here.
// Warning 6328: (387-401): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
