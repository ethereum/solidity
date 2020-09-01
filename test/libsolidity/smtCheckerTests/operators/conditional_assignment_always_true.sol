pragma experimental SMTChecker;

contract C {
	function f(bool b) public pure {
		require(b);
		uint c = b ? 5 : 1;
		assert(c < 5);
	}
}
// ----
// Warning 6328: (118-131): Assertion violation happens here
// Warning 6838: (105-106): Condition is always true.
