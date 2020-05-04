contract C {
	function f() public pure returns (bool) {
		uint8 x;
		assembly { x := 0x4242 }
		return (x == 0x42);
	}
}
// ====
// compileViaYul: also
// ----
// f() -> true
