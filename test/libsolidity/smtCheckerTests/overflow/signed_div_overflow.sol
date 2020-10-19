pragma experimental SMTChecker;

contract C  {
	function f(int x, int y) public pure returns (int) {
		return x / y;
	}
}
// ----
// Warning 4281: (110-115): CHC: Division by zero happens here.
// Warning 4984: (110-115): CHC: Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.
