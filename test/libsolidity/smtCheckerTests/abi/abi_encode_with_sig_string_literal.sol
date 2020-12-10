pragma experimental SMTChecker;
contract C {
	function abiEncodeStringLiteral(string memory sig) public pure {
		bytes memory b1 = abi.encodeWithSignature(sig, "");
		bytes memory b2 = abi.encodeWithSignature(sig, "");
		// should hold, but currently fails due to string literal abstraction
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encodeWithSignature(sig, bytes(""));
		assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encodeWithSignature(sig, bytes24(""));
		// should hold, but currently fails due to string literal abstraction
		assert(b1.length == b4.length);

		bytes memory b5 = abi.encodeWithSignature(sig, string(""));
		assert(b1.length == b5.length); // should fail

		bytes memory b6 = abi.encodeWithSelector("f()", bytes24(""));
		assert(b4.length == b6.length); // should fail
	}
}
// ----
// Warning 6328: (293-323): CHC: Assertion violation happens here.
// Warning 1218: (389-419): CHC: Error trying to invoke SMT solver.
// Warning 6328: (389-419): CHC: Assertion violation might happen here.
// Warning 1218: (574-604): CHC: Error trying to invoke SMT solver.
// Warning 6328: (574-604): CHC: Assertion violation might happen here.
// Warning 1218: (671-701): CHC: Error trying to invoke SMT solver.
// Warning 6328: (671-701): CHC: Assertion violation might happen here.
// Warning 1218: (785-815): CHC: Error trying to invoke SMT solver.
// Warning 6328: (785-815): CHC: Assertion violation might happen here.
// Warning 4661: (389-419): BMC: Assertion violation happens here.
// Warning 4661: (574-604): BMC: Assertion violation happens here.
// Warning 4661: (671-701): BMC: Assertion violation happens here.
// Warning 4661: (785-815): BMC: Assertion violation happens here.
