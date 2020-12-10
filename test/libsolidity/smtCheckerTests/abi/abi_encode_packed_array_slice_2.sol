pragma experimental SMTChecker;
contract C {
	function abiencodePackedSlice(uint[] calldata data) external pure {
		bytes memory b1 = abi.encodePacked(data);
		bytes memory b2 = abi.encodePacked(data[0:]);
		// should hold but the engine cannot infer that data is fully equals data[0:] because each index is assigned separately
		assert(b1.length == b2.length); // fails for now

		// Disabled because of Spacer nondeterminism
		//bytes memory b3 = abi.encodePacked(data[:data.length]);
		// should hold but the engine cannot infer that data is fully equals data[:data.length] because each index is assigned separately
		//assert(b1.length == b3.length); // fails for now

		bytes memory b4 = abi.encodePacked(data[5:10]);
		assert(b1.length == b4.length); // should fail

		uint x = 5;
		uint y = 10;
		bytes memory b5 = abi.encodePacked(data[x:y]);
		// should hold but the engine cannot infer that data[5:10] is fully equals data[x:y] because each index is assigned separately
		assert(b1.length == b5.length); // fails for now

		bytes memory b6 = abi.encode(data[5:10]);
		assert(b4.length == b6.length); // should fail
	}
}
// ----
// Warning 6328: (330-360): CHC: Assertion violation happens here.
// Warning 1218: (725-755): CHC: Error trying to invoke SMT solver.
// Warning 6328: (725-755): CHC: Assertion violation might happen here.
// Warning 1218: (982-1012): CHC: Error trying to invoke SMT solver.
// Warning 6328: (982-1012): CHC: Assertion violation might happen here.
// Warning 1218: (1078-1108): CHC: Error trying to invoke SMT solver.
// Warning 6328: (1078-1108): CHC: Assertion violation might happen here.
// Warning 4661: (725-755): BMC: Assertion violation happens here.
// Warning 4661: (982-1012): BMC: Assertion violation happens here.
// Warning 4661: (1078-1108): BMC: Assertion violation happens here.
