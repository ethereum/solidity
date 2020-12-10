pragma experimental SMTChecker;
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
// ----
// Warning 1218: (240-270): CHC: Error trying to invoke SMT solver.
// Warning 6328: (240-270): CHC: Assertion violation might happen here.
// Warning 1218: (318-348): CHC: Error trying to invoke SMT solver.
// Warning 6328: (318-348): CHC: Assertion violation might happen here.
// Warning 1218: (485-515): CHC: Error trying to invoke SMT solver.
// Warning 6328: (485-515): CHC: Assertion violation might happen here.
// Warning 1218: (564-594): CHC: Error trying to invoke SMT solver.
// Warning 6328: (564-594): CHC: Assertion violation might happen here.
// Warning 4661: (240-270): BMC: Assertion violation happens here.
// Warning 4661: (318-348): BMC: Assertion violation happens here.
// Warning 4661: (485-515): BMC: Assertion violation happens here.
// Warning 4661: (564-594): BMC: Assertion violation happens here.
