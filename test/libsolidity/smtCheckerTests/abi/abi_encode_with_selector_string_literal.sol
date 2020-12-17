pragma experimental SMTChecker;
contract C {
	function abiEncodeStringLiteral(bytes4 sel) public pure {
		bytes memory b1 = abi.encodeWithSelector(sel, "");
		bytes memory b2 = abi.encodeWithSelector(sel, "");
		// should hold, but currently fails due to string literal abstraction
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encodeWithSelector(sel, bytes(""));
		assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encodeWithSelector(sel, bytes24(""));
		// should hold, but currently fails due to string literal abstraction
		assert(b1.length == b4.length);

		bytes memory b5 = abi.encodeWithSelector(sel, string(""));
		assert(b1.length == b5.length); // should fail

		bytes memory b6 = abi.encodeWithSelector(0xcafecafe, bytes24(""));
		assert(b4.length == b6.length); // should fail
	}
}
// ----
// Warning 6328: (284-314): CHC: Assertion violation happens here.
// Warning 1218: (379-409): CHC: Error trying to invoke SMT solver.
// Warning 6328: (379-409): CHC: Assertion violation might happen here.
// Warning 1218: (563-593): CHC: Error trying to invoke SMT solver.
// Warning 6328: (563-593): CHC: Assertion violation might happen here.
// Warning 1218: (659-689): CHC: Error trying to invoke SMT solver.
// Warning 6328: (659-689): CHC: Assertion violation might happen here.
// Warning 1218: (778-808): CHC: Error trying to invoke SMT solver.
// Warning 6328: (778-808): CHC: Assertion violation might happen here.
// Warning 4661: (379-409): BMC: Assertion violation happens here.
// Warning 4661: (563-593): BMC: Assertion violation happens here.
// Warning 4661: (659-689): BMC: Assertion violation happens here.
// Warning 4661: (778-808): BMC: Assertion violation happens here.
