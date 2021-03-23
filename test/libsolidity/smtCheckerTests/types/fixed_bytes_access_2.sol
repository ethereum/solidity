pragma experimental SMTChecker;
contract C {
	function f(bytes calldata x, uint y) external pure {
		require(x.length > 10);
		x[8][0];
		// Disabled because of Spacer nondeterminism.
		//x[8][5%y];
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 5667: (75-81): Unused function parameter. Remove or comment out the variable name to silence this warning.
