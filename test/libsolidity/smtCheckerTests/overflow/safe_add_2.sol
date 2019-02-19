pragma experimental SMTChecker;

contract C
{
	function add(uint x, uint y) public pure returns (uint) {
		uint z = x + y;
		require(z >= x);
		return z;
	}
}
