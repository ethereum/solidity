pragma experimental SMTChecker;

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
// ----
// Warning 6328: (147-174): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\nb = 2\n\n\nTransaction trace:\nconstructor()\ntest(0, 2)
// Warning 6838: (332-348): BMC: Condition is always false.
