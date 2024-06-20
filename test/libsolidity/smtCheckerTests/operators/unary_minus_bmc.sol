contract C
{
	function f(int x) public pure {
		assert(x == -x);
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (48-63): BMC: Assertion violation happens here.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
