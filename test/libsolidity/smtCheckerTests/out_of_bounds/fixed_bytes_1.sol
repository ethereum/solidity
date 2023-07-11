contract C {
	function r(bytes4 x) public pure returns (bytes1) {
		return x[0]; // safe access
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
