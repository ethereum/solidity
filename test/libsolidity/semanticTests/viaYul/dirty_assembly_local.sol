contract C {
	function f() public pure returns (uint256) {
		uint8 x;
		uint256 y;
		assembly { x := 0x4242 }
		assembly { y := x }
		assert(y == 0x42);
		return y;
	}
}
// ====
// compileViaYul: true
// ----
// f() -> 0x42
