pragma experimental SMTChecker;

contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		require(x + y >= x);
		return x + y;
	}
}
// ----
// Warning 4984: (114-119): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
