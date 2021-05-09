contract C {
	uint constant x = 7;
	uint constant y = 3;
	uint constant z = x / y;

	function f() public pure {
		assert(z == 2);
		assert(z == x / 3);
		assert(z == 7 / y);
		assert(z * 3 != 7);
	}
}
// ====
// SMTEngine: all
// ----
