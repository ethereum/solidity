pragma experimental SMTChecker;
contract C {
	function mul(uint256 a, uint256 b) public pure returns (uint256) {
		if (a == 0) {
			return 0;
		}
		uint256 c = a * b;
		require(c / a == b);
		return c;
	}
}
// ----
// Warning 4984: (160-165): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
