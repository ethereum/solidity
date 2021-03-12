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
