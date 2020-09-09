pragma experimental SMTChecker;

contract C  {
	function f(int x, int y) public pure returns (int) {
		require(x >= y);
		return x - y;
	}
}
// ----
// Warning 4984: (129-134): Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.
