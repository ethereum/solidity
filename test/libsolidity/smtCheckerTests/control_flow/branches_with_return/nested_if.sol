contract C {

	function test(uint256 a, uint256 b) public pure {
		assert(nested_if(a,b) != 42); // should hold
		assert(nested_if(a,b) == 1);  // should fail
	}

	function nested_if(uint256 a, uint256 b) internal pure returns (uint256) {
		if (a < 5) {
			if (b > 1) {
				return 0;
			}
		}
		if (a == 2 && b == 2) {
			return 42; // unreachable
		}
		else {
			return 1;
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (114-141): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Warning 6838: (299-315): BMC: Condition is always false.
