contract C
{
	address thisAddr;
	function f(address a) public {
		require(a == address(this));
		thisAddr = a;
		assert(thisAddr == address(this));
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
