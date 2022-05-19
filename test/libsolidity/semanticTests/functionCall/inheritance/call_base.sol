contract Base {
	function f(uint n) public returns (uint) {
		return 2 * n;
	}
}

contract Child is Base {
	function g(uint n) public returns (uint) {
		return f(n);
	}
}
// ====
// compileToEwasm: also
// ----
// g(uint256): 4 -> 8
