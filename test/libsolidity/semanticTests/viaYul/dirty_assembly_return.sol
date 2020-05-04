contract C {
	function f() public pure returns (uint8 y) {
		assembly { y := 0x4242 }
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 0x42
