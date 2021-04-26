contract C {

	function test(uint256 a) public pure {
		assert(simple_if(a) == 1); // should fail for a == 0
	}

	function simple_if(uint256 a) internal pure returns (uint256) {
		if (a == 0) {
			return 0;
		}
		return 1;
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (56-81): BMC: Assertion violation happens here.
