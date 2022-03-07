contract C {
	uint[] b;

	function f() public {
		require(b.length == 0);
		b.push() = 1;
		assert(b[0] == 1);
	}

	function g() public {
		b.push() = 1;
		assert(b[b.length - 1] == 1);
		// Fails
		assert(b[b.length - 1] == 100);
	}

}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// SMTIgnoreCex: yes
// ----
// Warning 6328: (199-229): CHC: Assertion violation happens here.
