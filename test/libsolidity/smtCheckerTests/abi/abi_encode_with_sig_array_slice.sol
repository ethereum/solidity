pragma experimental SMTChecker;
contract C {
	function abiEncodeSlice(string memory sig, bytes calldata data) external pure {
		bytes memory b1 = abi.encodeWithSignature(sig, data);
		bytes memory b2 = abi.encodeWithSignature(sig, data[0:]);
		// should hold but the engine cannot infer that data is fully equals data[0:] because each index is assigned separately
		assert(b1.length == b2.length); // fails for now

		bytes memory b3 = abi.encodeWithSignature(sig, data[:data.length]);
		// should hold but the engine cannot infer that data is fully equals data[:data.length] because each index is assigned separately
		assert(b1.length == b3.length); // fails for now

		bytes memory b4 = abi.encodeWithSignature(sig, data[5:10]);
		assert(b1.length == b4.length); // should fail

		uint x = 5;
		uint y = 10;
		bytes memory b5 = abi.encodeWithSignature(sig, data[x:y]);
		// should hold but the engine cannot infer that data[5:10] is fully equals data[x:y] because each index is assigned separately
		assert(b1.length == b5.length); // fails for now

		bytes memory b6 = abi.encodeWithSelector("f()", data[5:10]);
		assert(b4.length == b6.length); // should fail
	}
}
// ----
// Warning 6328: (366-396): CHC: Assertion violation happens here.
// Warning 6328: (620-650): CHC: Assertion violation happens here.
// Warning 1218: (734-764): CHC: Error trying to invoke SMT solver.
// Warning 6328: (734-764): CHC: Assertion violation might happen here.
// Warning 1218: (1003-1033): CHC: Error trying to invoke SMT solver.
// Warning 6328: (1003-1033): CHC: Assertion violation might happen here.
// Warning 1218: (1118-1148): CHC: Error trying to invoke SMT solver.
// Warning 6328: (1118-1148): CHC: Assertion violation might happen here.
// Warning 4661: (734-764): BMC: Assertion violation happens here.
// Warning 4661: (1003-1033): BMC: Assertion violation happens here.
// Warning 4661: (1118-1148): BMC: Assertion violation happens here.
