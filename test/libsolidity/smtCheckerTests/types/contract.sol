contract C
{
	function f(C c, C d) public pure {
		assert(address(c) == address(d));
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (51-83): CHC: Assertion violation happens here.
