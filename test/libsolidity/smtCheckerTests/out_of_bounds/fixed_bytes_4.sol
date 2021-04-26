contract C {
	function r(bytes32 x, uint y) public pure {
		x[0]; // safe access
		x[y]; // oob access
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (83-87): CHC: Out of bounds access happens here.
