pragma experimental SMTChecker;

contract C
{
	function div(uint256 a, uint256 b) internal pure returns (uint256) {
		require(b > 0);
		uint256 c = a / b;
		return c;
	}
}
