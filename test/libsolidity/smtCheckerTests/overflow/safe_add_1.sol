pragma experimental SMTChecker;

contract C
{
	function add(uint x, uint y) public pure returns (uint) {
		require(x + y >= x);
		return x + y;
	}
}
