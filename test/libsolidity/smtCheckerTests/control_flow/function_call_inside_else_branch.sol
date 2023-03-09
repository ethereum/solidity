contract C
{
	function f() public pure {
		if (true) {
		} else {
			address a = g();
			assert(a == address(0));
		}
	}
	function g() public pure returns (address) {
		address x;
		x = address(0);
		return x;
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
