pragma experimental SMTChecker;

contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		return x / y;
	}
}
// ----
// Warning 3046: (113-118): BMC: Division by zero happens here.
