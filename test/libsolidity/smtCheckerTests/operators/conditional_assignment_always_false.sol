contract C {
	function f(uint b) public pure returns (uint d) {
		require(b < 10);
		uint c = b < 5 ? 5 : 1;
		d = c > 5 ? 3 : 2;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (115-120): BMC: Condition is always false.
