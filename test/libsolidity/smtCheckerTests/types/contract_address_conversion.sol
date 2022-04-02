contract C
{
	function f(C c, address a) public pure {
		assert(address(c) == a);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (57-80='assert(address(c) == a)'): CHC: Assertion violation happens here.
