contract C {

	function f() public pure returns (uint, uint, uint) {
		bytes memory a; bytes memory b; bytes memory c;
		(a, (b, c)) = ("0", ("1", "2"));
		return (uint8(a[0]), uint8(b[0]), uint8(c[0]));
	}

}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0x30, 0x31, 0x32
