pragma experimental SMTChecker;

contract C{
    uint x;
	constructor(uint y) {
		assert(x == 1);
		x = 1;
	}
    function f() public {
		assert(x == 2);
		++x;
		g();
		assert(x == 2);
    }

	function g() internal {
		assert(x == 3);
		--x;
		assert(x == 2);
	}
}
// ----
// Warning 5667: (70-76): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (138-152): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nconstructor(0)\nState: x = 1\nf()
// Warning 6328: (170-184): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nconstructor(0)\nState: x = 1\nf()
// Warning 6328: (220-234): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\n\n\nTransaction trace:\nconstructor(0)\nState: x = 1\nf()
// Warning 6328: (245-259): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nconstructor(0)\nState: x = 1\nf()
// Warning 6328: (82-96): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\ny = 0\n\n\nTransaction trace:\nconstructor(0)
