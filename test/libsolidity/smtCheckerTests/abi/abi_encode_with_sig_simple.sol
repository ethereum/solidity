pragma experimental SMTChecker;
contract C {
	function abiEncodeSimple(string memory sig, bool t, uint x, uint y, uint z, uint[] memory a, uint[] memory b) public pure {
		require(x == y);
		bytes memory b1 = abi.encodeWithSignature(sig, x, z, a);
		bytes memory b2 = abi.encodeWithSignature(sig, y, z, a);
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encodeWithSignature(sig, y, z, b);
		assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encodeWithSignature(sig, t, z, a);
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encodeWithSignature(sig, y, y, y, y, a, a, a);
		assert(b1.length != b5.length); // should fail
		assert(b1.length == b5.length); // should fail

		bytes memory b6 = abi.encodeWithSignature("f()", x, z, a);
		assert(b1.length == b6.length); // should fail
	}
}
// ----
// Warning 6328: (403-433): CHC: Assertion violation happens here.
// Warning 6328: (512-542): CHC: Assertion violation happens here.
// Warning 1218: (633-663): CHC: Error trying to invoke SMT solver.
// Warning 6328: (633-663): CHC: Assertion violation might happen here.
// Warning 1218: (682-712): CHC: Error trying to invoke SMT solver.
// Warning 6328: (682-712): CHC: Assertion violation might happen here.
// Warning 1218: (793-823): CHC: Error trying to invoke SMT solver.
// Warning 6328: (793-823): CHC: Assertion violation might happen here.
// Warning 4661: (633-663): BMC: Assertion violation happens here.
// Warning 4661: (682-712): BMC: Assertion violation happens here.
// Warning 4661: (793-823): BMC: Assertion violation happens here.
