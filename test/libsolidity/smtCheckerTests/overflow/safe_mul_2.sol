pragma experimental SMTChecker;

contract C
{
	function mul(uint256 a, uint256 b) internal pure returns (uint256) {
		if (a == 0) {
			return 0;
		}
		uint256 c = a * b;
		require(c / a == b);
		return c;
	}
}
// ----
// Warning: (180-185): Division by zero happens here
