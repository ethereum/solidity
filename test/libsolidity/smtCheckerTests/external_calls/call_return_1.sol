contract C {
	uint x;
	function f(address a) public {
		(bool s, bytes memory data) = a.call("");
		assert(s); // should fail
		assert(!s); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (65-82='bytes memory data'): Unused local variable.
// Warning 6328: (100-109='assert(s)'): CHC: Assertion violation happens here.
// Warning 6328: (128-138='assert(!s)'): CHC: Assertion violation happens here.
