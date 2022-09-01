contract A {
	function f() public pure returns (uint, uint, uint, uint) {
		uint x = 3;
		uint y = 1;
		uint z = (x > y) ? x : y;
		uint w = x < y ? x : y;
		return (x, y, z, w);
	}
}
// ====
// compileToEwasm: also
// ----
// f() -> 3, 1, 3, 1
