contract C
{
	function f(uint x) public pure returns (uint, uint) {
		return (x, x);
	}

	function g() public pure {
		(uint a, uint b) = f(0);
		(uint c, uint d) = f(0);
		assert(a == c && b == d);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
