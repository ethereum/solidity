contract C {
	uint x;
	function f(address a) public {
		(bool s, bytes memory data) = a.call("");
		assert(data.length > 10); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (57-63): Unused local variable.
// Warning 6328: (100-124): CHC: Assertion violation happens here.
