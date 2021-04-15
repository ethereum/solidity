contract C {
	function f() public pure {
		bytes memory y = "def";
		y[0] &= "d";
		assert(y[0] == "d");

		y[0] |= "e";
		assert(y[0] == "d"); // fails

		y[0] ^= "f";
		// Disabled because of nondeterminism in Spacer in Z3 4.8.9
		//assert(y[0] == (bytes1("d") | bytes1("e")) ^ bytes1("f"));
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (123-142): CHC: Assertion violation happens here.
