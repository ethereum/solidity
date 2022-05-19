contract A {
	uint8 immutable a = 2;
	function f() public view returns (uint) {
		return a;
	}
}
// ----
// f() -> 2
