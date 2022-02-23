contract C {
	function abiEncodeStringLiteral() public pure {
		bytes memory b1 = abi.encode("");
		bytes memory b2 = abi.encode("");
		// should hold, but currently fails due to string literal abstraction
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encode(bytes(""));
		assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encode(bytes24(""));
		// should hold, but currently fails due to string literal abstraction
		assert(b1.length == b4.length);

		bytes memory b5 = abi.encode(string(""));
		assert(b1.length == b5.length); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (208-238): CHC: Error trying to invoke SMT solver.
// Warning 1218: (286-316): CHC: Error trying to invoke SMT solver.
// Warning 1218: (453-483): CHC: Error trying to invoke SMT solver.
// Warning 1218: (532-562): CHC: Error trying to invoke SMT solver.
// Warning 6328: (208-238): CHC: Assertion violation might happen here.
// Warning 6328: (286-316): CHC: Assertion violation might happen here.
// Warning 6328: (453-483): CHC: Assertion violation might happen here.
// Warning 6328: (532-562): CHC: Assertion violation might happen here.
// Warning 4661: (208-238): BMC: Assertion violation happens here.
// Warning 4661: (286-316): BMC: Assertion violation happens here.
// Warning 4661: (453-483): BMC: Assertion violation happens here.
// Warning 4661: (532-562): BMC: Assertion violation happens here.
