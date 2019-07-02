pragma experimental SMTChecker;
contract C {
	function f(uint a) public pure {
		require(a < 10, "Input number is too large.");
		assert(a < 20);
	}
}
// ----
// Warning: (97-125): Assertion checker does not yet support the type of this literal (literal_string "Input number is too large.").
