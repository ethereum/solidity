contract C{
    uint x;
	constructor(uint y) {
		assert(x == 0);
		x = 1;
	}
    function f() public {
		assert(x == 1);
		++x;
		g();
		assert(x == 1);
    }

	function g() internal {
		assert(x == 2);
		--x;
		assert(x == 1);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 5667: (37-43): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Info 1391: CHC: 7 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
