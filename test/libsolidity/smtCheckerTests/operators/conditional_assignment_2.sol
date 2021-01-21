pragma experimental SMTChecker;

contract C {
	function f(uint b) public pure {
		require(b < 3);
		uint c = (b > 0) ? b++ : ++b;
		assert(c == 0);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (132-146): CHC: Assertion violation happens here.
