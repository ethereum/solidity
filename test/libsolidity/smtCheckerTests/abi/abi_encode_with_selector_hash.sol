contract C {
	function abiEncodeHash(bytes4 sel, uint a, uint b) public pure {
		require(a == b);
		bytes memory b1 = abi.encodeWithSelector(sel, a, a, a, a);
		bytes memory b2 = abi.encodeWithSelector(sel, b, a, b, a);
		assert(keccak256(b1) == keccak256(b2));

		bytes memory b3 = abi.encodeWithSelector(0xcafecafe, a, a, a, a);
		assert(keccak256(b1) == keccak256(b3)); // should fail
		assert(keccak256(b1) != keccak256(b3)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (333-371): CHC: Error trying to invoke SMT solver.
// Warning 6328: (333-371): CHC: Assertion violation might happen here.
// Warning 1218: (390-428): CHC: Error trying to invoke SMT solver.
// Warning 6328: (390-428): CHC: Assertion violation might happen here.
// Warning 4661: (333-371): BMC: Assertion violation happens here.
// Warning 4661: (390-428): BMC: Assertion violation happens here.
