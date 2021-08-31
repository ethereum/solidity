contract C {
	function abiEncodeSimple(bytes4 sel, bool t, uint x, uint y, uint z, uint[] memory a, uint[] memory b) public pure {
		require(x == y);
		// Disabled because of Spacer nondeterminism
		bytes memory b1 = abi.encodeWithSelector(sel, x, z, a);
		//bytes memory b2 = abi.encodeWithSelector(sel, y, z, a);
		//assert(b1.length == b2.length);

		// Disabled because of Spacer nondeterminism
		//bytes memory b3 = abi.encodeWithSelector(sel, y, z, b);
		//assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encodeWithSelector(sel, t, z, a);
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encodeWithSelector(sel, y, y, y, y, a, a, a);
		assert(b1.length != b5.length); // should fail
		assert(b1.length == b5.length); // should fail

		bytes memory b6 = abi.encodeWithSelector(0xcafecafe, x, z, a);
		assert(b1.length == b6.length); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 5667: (100-115): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 1218: (855-885): CHC: Error trying to invoke SMT solver.
// Warning 6328: (571-601): CHC: Assertion violation happens here.
// Warning 6328: (691-721): CHC: Assertion violation happens here.
// Warning 6328: (740-770): CHC: Assertion violation happens here.
// Warning 6328: (855-885): CHC: Assertion violation might happen here.
// Warning 4661: (855-885): BMC: Assertion violation happens here.
