contract C
{
	function f(address a) public view {
		assert(a == address(this));
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (52-78): CHC: Assertion violation happens here.
