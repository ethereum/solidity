contract C {
	function abiEncodeHash(bytes4 sel, uint a, uint b) public pure {
		require(a == b);
		bytes memory b1 = abi.encodeWithSelector(sel, a, a, a, a);
		bytes memory b2 = abi.encodeWithSelector(sel, b, a, b, a);
		// Disabled because of OSX nondeterminism
		//assert(keccak256(b1) == keccak256(b2));

		bytes memory b3 = abi.encodeWithSelector(0xcafecafe, a, a, a, a);
		assert(keccak256(b1) == keccak256(b3)); // should fail
		assert(keccak256(b1) != keccak256(b3)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (161-176): Unused local variable.
// Warning 1218: (379-417): CHC: Error trying to invoke SMT solver.
// Warning 1218: (436-474): CHC: Error trying to invoke SMT solver.
// Warning 6328: (379-417): CHC: Assertion violation might happen here.
// Warning 6328: (436-474): CHC: Assertion violation might happen here.
// Warning 4661: (379-417): BMC: Assertion violation happens here.
// Warning 4661: (436-474): BMC: Assertion violation happens here.
