contract A {
	function f() public pure returns (uint) {
		uint x = 3 < 0 ? 2 > 1 ? 2 : 1 : 7 > 2 ? 7 : 6;
		return x;
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 7
