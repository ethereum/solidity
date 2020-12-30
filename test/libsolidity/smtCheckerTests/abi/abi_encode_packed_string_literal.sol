pragma experimental SMTChecker;
contract C {
	function abiencodePackedStringLiteral() public pure {
		bytes memory b1 = abi.encodePacked("");
		bytes memory b2 = abi.encodePacked("");
		// should hold, but currently fails due to string literal abstraction
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encodePacked(bytes(""));
		assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encodePacked(bytes24(""));
		// should hold, but currently fails due to string literal abstraction
		assert(b1.length == b4.length);

		bytes memory b5 = abi.encodePacked(string(""));
		assert(b1.length == b5.length); // should fail

		bytes memory b6 = abi.encode("");
		assert(b1.length == b6.length); // should fail
	}
}
// ----
// Warning 6328: (258-288): CHC: Assertion violation happens here.
// Warning 1218: (342-372): CHC: Error trying to invoke SMT solver.
// Warning 6328: (342-372): CHC: Assertion violation might happen here.
// Warning 1218: (515-545): CHC: Error trying to invoke SMT solver.
// Warning 6328: (515-545): CHC: Assertion violation might happen here.
// Warning 1218: (600-630): CHC: Error trying to invoke SMT solver.
// Warning 6328: (600-630): CHC: Assertion violation might happen here.
// Warning 1218: (686-716): CHC: Error trying to invoke SMT solver.
// Warning 6328: (686-716): CHC: Assertion violation might happen here.
// Warning 4661: (342-372): BMC: Assertion violation happens here.
// Warning 4661: (515-545): BMC: Assertion violation happens here.
// Warning 4661: (600-630): BMC: Assertion violation happens here.
// Warning 4661: (686-716): BMC: Assertion violation happens here.
