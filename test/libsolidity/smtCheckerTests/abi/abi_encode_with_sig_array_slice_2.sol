pragma experimental SMTChecker;
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
// ----
// Warning 6328: (367-397): CHC: Assertion violation happens here.
// Warning 6328: (621-651): CHC: Assertion violation happens here.
// Warning 1218: (735-765): CHC: Error trying to invoke SMT solver.
// Warning 6328: (735-765): CHC: Assertion violation might happen here.
// Warning 1218: (1004-1034): CHC: Error trying to invoke SMT solver.
// Warning 6328: (1004-1034): CHC: Assertion violation might happen here.
// Warning 1218: (1119-1149): CHC: Error trying to invoke SMT solver.
// Warning 6328: (1119-1149): CHC: Assertion violation might happen here.
// Warning 4661: (735-765): BMC: Assertion violation happens here.
// Warning 4661: (1004-1034): BMC: Assertion violation happens here.
// Warning 4661: (1119-1149): BMC: Assertion violation happens here.
