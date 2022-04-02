contract C
{
	function f(C c, C d) public pure {
		assert(c == d);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (51-65='assert(c == d)'): CHC: Assertion violation happens here.
