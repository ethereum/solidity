contract C {
	function r(bytes4 x) public pure returns (bytes1) {
		return x[0]; // safe access
	}
}
// ====
// SMTEngine: all
// ----
