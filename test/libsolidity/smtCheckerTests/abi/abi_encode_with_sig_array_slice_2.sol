contract C {
	function abiEncodeSlice(string memory sig, uint[] calldata data) external pure {
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
// ====
// SMTEngine: all
// ----
// Warning 6328: (335-365): CHC: Assertion violation happens here.
// Warning 6328: (589-619): CHC: Assertion violation happens here.
// Warning 1218: (703-733): CHC: Error trying to invoke SMT solver.
// Warning 6328: (703-733): CHC: Assertion violation might happen here.
// Warning 1218: (972-1002): CHC: Error trying to invoke SMT solver.
// Warning 6328: (972-1002): CHC: Assertion violation might happen here.
// Warning 1218: (1087-1117): CHC: Error trying to invoke SMT solver.
// Warning 6328: (1087-1117): CHC: Assertion violation might happen here.
// Warning 4661: (703-733): BMC: Assertion violation happens here.
// Warning 4661: (972-1002): BMC: Assertion violation happens here.
// Warning 4661: (1087-1117): BMC: Assertion violation happens here.
