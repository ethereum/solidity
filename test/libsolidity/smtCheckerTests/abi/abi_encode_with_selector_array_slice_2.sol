contract C {
	function abiEncodeSlice(bytes4 sel, uint[] calldata data) external pure {
		bytes memory b1 = abi.encodeWithSelector(sel, data);
		bytes memory b2 = abi.encodeWithSelector(sel, data[0:]);
		// should hold but the engine cannot infer that data is fully equals data[0:] because each index is assigned separately
		assert(b1.length == b2.length); // fails for now

		bytes memory b3 = abi.encodeWithSelector(sel, data[:data.length]);
		// should hold but the engine cannot infer that data is fully equals data[:data.length] because each index is assigned separately
		assert(b1.length == b3.length); // fails for now

		bytes memory b4 = abi.encodeWithSelector(sel, data[5:10]);
		assert(b1.length == b4.length); // should fail

		uint x = 5;
		uint y = 10;
		bytes memory b5 = abi.encodeWithSelector(sel, data[x:y]);
		// should hold but the engine cannot infer that data[5:10] is fully equals data[x:y] because each index is assigned separately
		assert(b1.length == b5.length); // fails for now

		bytes memory b6 = abi.encodeWithSelector(0xcafecafe, data[5:10]);
		assert(b4.length == b6.length); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: yes
// SMTIgnoreOS: macos
// ----
// Warning 6328: (326-356): CHC: Assertion violation happens here.
// Warning 6328: (579-609): CHC: Assertion violation happens here.
// Warning 6328: (692-722): CHC: Assertion violation happens here.
// Warning 6328: (960-990): CHC: Assertion violation happens here.
// Warning 6328: (1080-1110): CHC: Assertion violation happens here.
