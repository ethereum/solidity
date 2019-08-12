pragma experimental SMTChecker;
contract C {
	function f(uint a) public pure {
		require(a < 10, "Input number is too large.");
		assert(a < 20);
	}
}
// ----
