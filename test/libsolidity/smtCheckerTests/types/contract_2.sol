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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (76-90): CHC: Assertion violation happens here.
