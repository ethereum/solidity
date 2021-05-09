contract C {

	function f(bytes1 b) public pure {
		bytes1 c = hex"7f";
		require(b > c);
		assert(uint8(b) > 127); // should hold
	}
}
// ====
// SMTEngine: all
// ----
