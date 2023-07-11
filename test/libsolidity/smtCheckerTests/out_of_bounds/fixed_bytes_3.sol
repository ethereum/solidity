contract C {
	function r(bytes32 x, uint y) public pure {
		require(y <= 31);
		x[0]; // safe access
		x[y]; // safe access
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
