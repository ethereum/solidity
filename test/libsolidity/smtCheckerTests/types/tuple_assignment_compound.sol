contract C
{
	function f() public pure {
		uint a = 1;
		uint b = 3;
		a += ((((b))));
		assert(a == 3);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (89-103): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
