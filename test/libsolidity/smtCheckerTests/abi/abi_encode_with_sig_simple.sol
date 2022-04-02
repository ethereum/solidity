contract C {
	function abiEncodeSimple(string memory sig, bool t, uint x, uint y, uint z, uint[] memory a, uint[] memory b) public pure {
		require(x == y);
		bytes memory b1 = abi.encodeWithSignature(sig, x, z, a);
		bytes memory b2 = abi.encodeWithSignature(sig, y, z, a);
		assert(b1.length == b2.length);

		// Disabled because of nondeterminism in Spacer Z3 4.8.9
		//bytes memory b3 = abi.encodeWithSignature(sig, y, z, b);
		//assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encodeWithSignature(sig, t, z, a);
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encodeWithSignature(sig, y, y, y, y, a, a, a);
		assert(b1.length != b5.length); // should fail
		assert(b1.length == b5.length); // should fail

		bytes memory b6 = abi.encodeWithSignature("f()", x, z, a);
		assert(b1.length == b6.length); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 5667: (107-122='uint[] memory b'): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 1218: (824-854='assert(b1.length == b6.length)'): CHC: Error trying to invoke SMT solver.
// Warning 6328: (543-573='assert(b1.length == b4.length)'): CHC: Assertion violation happens here.
// Warning 6328: (664-694='assert(b1.length != b5.length)'): CHC: Assertion violation happens here.
// Warning 6328: (713-743='assert(b1.length == b5.length)'): CHC: Assertion violation happens here.
// Warning 6328: (824-854='assert(b1.length == b6.length)'): CHC: Assertion violation might happen here.
// Warning 4661: (824-854='assert(b1.length == b6.length)'): BMC: Assertion violation happens here.
