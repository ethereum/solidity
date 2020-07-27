pragma experimental SMTChecker;

contract C {

	function test() public pure {
		assert(branches(0) == 0);
		assert(branches(1) == 42);
	}

	function branches(uint256 a) internal pure returns (uint256) {
		if (a == 0) {
			return 0;
		}
		else {
			return 42;
		}
		return 1; // dead code
	}
}
// ----
// Warning 5740: (265-273): Unreachable code.
