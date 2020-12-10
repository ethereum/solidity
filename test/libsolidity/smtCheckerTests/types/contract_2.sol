pragma experimental SMTChecker;

contract D
{
	uint x;
}

contract C
{
	function f(D c, D d) public pure {
		assert(c == d);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (109-123): CHC: Assertion violation happens here.
