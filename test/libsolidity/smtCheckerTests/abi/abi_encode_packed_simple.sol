contract C {
	function abiencodePackedSimple(bool t, uint x, uint y, uint z, uint[] memory a, uint[] memory b) public pure {
		require(x == y);
		bytes memory b1 = abi.encodePacked(x, z, a);
		bytes memory b2 = abi.encodePacked(y, z, a);
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encodePacked(y, z, b);
		assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encodePacked(t, z, a);
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encodePacked(y, y, y, y, a, a, a);
		assert(b1.length != b5.length); // should fail
		assert(b1.length == b5.length); // should fail

		// Commented out because of nondeterminism in Spacer in Z3 4.8.9
		//bytes memory b6 = abi.encode(x, z, a);
		//assert(b1.length == b6.length); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (322-352): CHC: Error trying to invoke SMT solver.
// Warning 1218: (419-449): CHC: Error trying to invoke SMT solver.
// Warning 1218: (528-558): CHC: Error trying to invoke SMT solver.
// Warning 1218: (577-607): CHC: Error trying to invoke SMT solver.
// Warning 6328: (322-352): CHC: Assertion violation might happen here.
// Warning 6328: (419-449): CHC: Assertion violation might happen here.
// Warning 6328: (528-558): CHC: Assertion violation might happen here.
// Warning 6328: (577-607): CHC: Assertion violation might happen here.
// Warning 4661: (322-352): BMC: Assertion violation happens here.
// Warning 4661: (419-449): BMC: Assertion violation happens here.
// Warning 4661: (528-558): BMC: Assertion violation happens here.
// Warning 4661: (577-607): BMC: Assertion violation happens here.
