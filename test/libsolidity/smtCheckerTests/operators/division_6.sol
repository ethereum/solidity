pragma experimental SMTChecker;
contract C {
	function mul(uint256 a, uint256 b) internal pure returns (uint256) {
		if (a == 0) {
			return 0;
		}
		// TODO remove when SMTChecker sees that this code is the `else` of the `return`.
		require(a != 0);
		uint256 c = a * b;
		require(c / a == b);
		return c;
	}
}
