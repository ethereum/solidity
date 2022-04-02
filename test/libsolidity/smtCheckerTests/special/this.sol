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
// Warning 6328: (52-78='assert(a == address(this))'): CHC: Assertion violation happens here.
