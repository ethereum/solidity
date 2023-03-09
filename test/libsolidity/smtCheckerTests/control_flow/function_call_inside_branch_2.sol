contract C
{
	function f() public pure {
		if (true) {
			address a = g();
			assert(a == address(0));
		}
		else
		{
			address b = g();
			assert(b == address(0));
		}
	}
	function g() public pure returns (address) {
		address a;
		a = address(0);
		return a;
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
