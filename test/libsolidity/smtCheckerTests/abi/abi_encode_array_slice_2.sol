pragma experimental SMTChecker;
contract C {
	function abiEncodeSlice(uint[] calldata data) external pure {
		bytes memory b1 = abi.encode(data);
		bytes memory b2 = abi.encode(data[0:]);
		// should hold but the engine cannot infer that data is fully equals data[0:] because each index is assigned separately
		assert(b1.length == b2.length); // fails for now

		bytes memory b3 = abi.encode(data[:data.length]);
		// should hold but the engine cannot infer that data is fully equals data[:data.length] because each index is assigned separately
		assert(b1.length == b3.length); // fails for now

		bytes memory b4 = abi.encode(data[5:10]);
		assert(b1.length == b4.length); // should fail

		uint x = 5;
		uint y = 10;
		bytes memory b5 = abi.encode(data[x:y]);
		// should hold but the engine cannot infer that data[5:10] is fully equals data[x:y] because each index is assigned separately
		assert(b1.length == b5.length); // fails for now
	}
}
// ----
// Warning 6328: (312-342): CHC: Assertion violation happens here.
// Warning 6328: (548-578): CHC: Assertion violation happens here.
// Warning 1218: (644-674): CHC: Error trying to invoke SMT solver.
// Warning 6328: (644-674): CHC: Assertion violation might happen here.
// Warning 1218: (895-925): CHC: Error trying to invoke SMT solver.
// Warning 6328: (895-925): CHC: Assertion violation might happen here.
// Warning 4661: (644-674): BMC: Assertion violation happens here.
// Warning 4661: (895-925): BMC: Assertion violation happens here.
