contract C
{
	function f(int x) public pure {
		assert(x == -x);
	}
}
// ====
// SMTEngine: chc
// ----
//  Warning 6328: (48-63): CHC: Assertion violation happens here.
//  Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
