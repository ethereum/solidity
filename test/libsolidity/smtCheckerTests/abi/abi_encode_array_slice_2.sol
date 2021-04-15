contract C {
	function abiEncodeSlice(uint[] calldata data) external pure {
		bytes memory b1 = abi.encode(data);
		bytes memory b2 = abi.encode(data[0:]);
		// should hold but the engine cannot infer that data is fully equals data[0:] because each index is assigned separately
		assert(b1.length == b2.length); // fails for now

		bytes memory b3 = abi.encode(data[:data.length]);
		// should hold but the engine cannot infer that data is fully equals data[:data.length] because each index is assigned separately
		// Disabled because of Spacer nondeterminism.
		//assert(b1.length == b3.length); // fails for now

		bytes memory b4 = abi.encode(data[5:10]);
		// Disabled because of Spacer nondeterminism.
		//assert(b1.length == b4.length); // should fail

		uint x = 5;
		uint y = 10;
		bytes memory b5 = abi.encode(data[x:y]);
		// should hold but the engine cannot infer that data[5:10] is fully equals data[x:y] because each index is assigned separately
		// Disabled because of Spacer nondeterminism.
		//assert(b1.length == b5.length); // fails for now
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (332-347): Unused local variable.
// Warning 2072: (618-633): Unused local variable.
// Warning 2072: (791-806): Unused local variable.
// Warning 6328: (280-310): CHC: Assertion violation happens here.
