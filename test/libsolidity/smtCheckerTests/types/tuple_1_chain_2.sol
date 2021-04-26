contract C {
	function i() public pure returns (uint d) {
		if (0==0)
			((d)) = 13;
		assert(d == 13);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (64-68): BMC: Condition is always true.
