pragma experimental SMTChecker;

contract C
{
	function mul(uint256 a, uint256 b) internal pure returns (uint256) {
		uint256 c;
		if (a != 0) {
			c = a * b;
			require(c / a == b);
		}
		return c;
	}
}
