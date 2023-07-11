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
// ====
// SMTEngine: all
// ----
// Warning 5667: (37-43): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (49-63): CHC: Assertion violation happens here.
// Warning 6328: (105-119): CHC: Assertion violation happens here.
// Warning 6328: (137-151): CHC: Assertion violation happens here.
// Warning 6328: (187-201): CHC: Assertion violation happens here.
// Warning 6328: (212-226): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
