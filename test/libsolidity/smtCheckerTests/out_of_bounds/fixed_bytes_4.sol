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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
