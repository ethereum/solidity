pragma experimental SMTChecker;
contract C {
	function f(uint x, uint y) public pure returns (uint) {
		return x / y;
	}
}
// ----
// Warning: (111-116): Division by zero happens here
