contract C {
	function r(bytes32 x, uint y) public pure {
		x[0]; // safe access
		// Disabled because of Spacer nondeterminism.
		//x[y]; // oob access
	}
}
// ====
// SMTEngine: all
// ----
// Warning 5667: (36-42): Unused function parameter. Remove or comment out the variable name to silence this warning.
