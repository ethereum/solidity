pragma experimental SMTChecker;

contract C  {
	function f(int x, int y) public pure returns (int) {
		return x % y;
	}
}
// ----
// Warning 3046: (110-115): BMC: Division by zero happens here.
