contract C {
	function f() public pure {
		bytes memory x = hex"0123";
		assert(x.length == 2);
	}
	function g() public pure {
		bytes memory x = bytes(hex"0123");
		assert(x.length == 2);
	}
}
// ====
// SMTEngine: all
// ----
