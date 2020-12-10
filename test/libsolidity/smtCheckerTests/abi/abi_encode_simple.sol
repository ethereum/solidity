pragma experimental SMTChecker;
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
		assert(b1.length == b5.length); // should fail
	}
}
// ----
// Warning 1218: (330-360): CHC: Error trying to invoke SMT solver.
// Warning 6328: (330-360): CHC: Assertion violation might happen here.
// Warning 1218: (421-451): CHC: Error trying to invoke SMT solver.
// Warning 6328: (421-451): CHC: Assertion violation might happen here.
// Warning 1218: (524-554): CHC: Error trying to invoke SMT solver.
// Warning 6328: (524-554): CHC: Assertion violation might happen here.
// Warning 6328: (573-603): CHC: Assertion violation happens here.
// Warning 4661: (330-360): BMC: Assertion violation happens here.
// Warning 4661: (421-451): BMC: Assertion violation happens here.
// Warning 4661: (524-554): BMC: Assertion violation happens here.
