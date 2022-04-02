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
// Warning 1218: (208-238='assert(b1.length == b2.length)'): CHC: Error trying to invoke SMT solver.
// Warning 1218: (286-316='assert(b1.length == b3.length)'): CHC: Error trying to invoke SMT solver.
// Warning 1218: (453-483='assert(b1.length == b4.length)'): CHC: Error trying to invoke SMT solver.
// Warning 1218: (532-562='assert(b1.length == b5.length)'): CHC: Error trying to invoke SMT solver.
// Warning 6328: (208-238='assert(b1.length == b2.length)'): CHC: Assertion violation might happen here.
// Warning 6328: (286-316='assert(b1.length == b3.length)'): CHC: Assertion violation might happen here.
// Warning 6328: (453-483='assert(b1.length == b4.length)'): CHC: Assertion violation might happen here.
// Warning 6328: (532-562='assert(b1.length == b5.length)'): CHC: Assertion violation might happen here.
// Warning 4661: (208-238='assert(b1.length == b2.length)'): BMC: Assertion violation happens here.
// Warning 4661: (286-316='assert(b1.length == b3.length)'): BMC: Assertion violation happens here.
// Warning 4661: (453-483='assert(b1.length == b4.length)'): BMC: Assertion violation happens here.
// Warning 4661: (532-562='assert(b1.length == b5.length)'): BMC: Assertion violation happens here.
