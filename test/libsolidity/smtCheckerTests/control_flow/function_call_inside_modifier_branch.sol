contract C
{
	modifier m(address a) {
		if (true) {
			a = g();
			_;
			assert(a == address(0));
		}
	}

	function f(address a) m(a) public pure {
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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
