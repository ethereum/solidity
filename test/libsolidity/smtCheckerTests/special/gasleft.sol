contract C
{
	function f() public view {
		assert(gasleft() > 0);
		uint g = gasleft();
		assert(g < gasleft());
		assert(g >= gasleft());
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (43-64): CHC: Assertion violation happens here.
// Warning 6328: (90-111): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
