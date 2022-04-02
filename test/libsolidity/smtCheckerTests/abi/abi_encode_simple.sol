contract C {
	function abiEncodeSimple(bool t, uint x, uint y, uint z, uint[] memory a, uint[] memory b) public pure {
		require(x == y);
		bytes memory b1 = abi.encode(x, z, a);
		bytes memory b2 = abi.encode(y, z, a);
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encode(y, z, b);
		assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encode(t, z, a);
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encode(y, y, y, y, a, a, a);
		assert(b1.length != b5.length); // should fail
		// Disabled because of nondeterminism in Spacer Z3 4.8.9
		//assert(b1.length == b5.length); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (298-328='assert(b1.length == b3.length)'): CHC: Error trying to invoke SMT solver.
// Warning 1218: (389-419='assert(b1.length == b4.length)'): CHC: Error trying to invoke SMT solver.
// Warning 1218: (492-522='assert(b1.length != b5.length)'): CHC: Error trying to invoke SMT solver.
// Warning 6328: (298-328='assert(b1.length == b3.length)'): CHC: Assertion violation might happen here.
// Warning 6328: (389-419='assert(b1.length == b4.length)'): CHC: Assertion violation might happen here.
// Warning 6328: (492-522='assert(b1.length != b5.length)'): CHC: Assertion violation might happen here.
// Warning 4661: (298-328='assert(b1.length == b3.length)'): BMC: Assertion violation happens here.
// Warning 4661: (389-419='assert(b1.length == b4.length)'): BMC: Assertion violation happens here.
// Warning 4661: (492-522='assert(b1.length != b5.length)'): BMC: Assertion violation happens here.
