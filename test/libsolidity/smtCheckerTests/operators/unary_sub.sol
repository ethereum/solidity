contract C
{
	function f() public pure {
		uint x = 5;
		uint a = --x;
		assert(x == 4);
		assert(a == 4);
		uint b = x--;
		assert(x == 3);
		// Should fail.
		assert(b > 4);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (161-174): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
