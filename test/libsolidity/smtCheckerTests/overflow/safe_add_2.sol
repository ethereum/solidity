pragma experimental SMTChecker;

contract C
{
	function add(uint x, uint y) public pure returns (uint) {
		uint z = x + y;
		require(z >= x);
		return z;
	}
}
// ----
// Warning 4984: (116-121): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
