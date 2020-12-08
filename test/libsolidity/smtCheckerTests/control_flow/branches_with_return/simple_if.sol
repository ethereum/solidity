pragma experimental SMTChecker;

contract C {
	function test(uint256 a, uint256 b) public pure returns (uint256) {
		if (a == 0) {
			return 0;
		}
		return b / a; // This division is safe because of the early return in if-block.
	}
}
