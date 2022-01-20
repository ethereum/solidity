contract D
{
	uint x;
}

contract C
{
	function f(D c, D d) public pure {
		assert(address(c) == address(d));
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (76-108): CHC: Assertion violation happens here.
