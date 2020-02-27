contract A {
	function f() public pure returns (uint) {
		uint x = true ? 1 : 0;
		uint y = false ? 0 : 1;
		return x + y;
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 2
