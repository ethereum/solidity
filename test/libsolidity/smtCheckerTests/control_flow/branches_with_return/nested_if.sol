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
// Warning 6328: (114-141): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\nb = 2\n\nTransaction trace:\nC.constructor()\nC.test(0, 2)\n    C.nested_if(0, 2) -- internal call\n    C.nested_if(0, 2) -- internal call
// Warning 6838: (299-315): BMC: Condition is always false.
