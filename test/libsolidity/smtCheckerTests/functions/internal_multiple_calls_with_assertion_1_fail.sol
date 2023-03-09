contract C{
    uint x;
	constructor(uint y) {
		assert(x == 1);
		x = 1;
	}
    function f() public {
		assert(x == 2);
		++x;
		++x;
		g();
		g();
		assert(x == 3);
    }

	function g() internal {
		--x;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 5667: (37-43): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (49-63): CHC: Assertion violation happens here.
// Warning 6328: (105-119): CHC: Assertion violation happens here.
// Warning 6328: (151-165): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
