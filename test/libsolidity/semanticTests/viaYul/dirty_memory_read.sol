contract C {
	function f() public pure returns (uint8 x, bool a, bool b) {
		uint8[1] memory m;
		assembly {
			mstore(m, 257)
		}
		x = m[0];
		a = (m[0] == 0x01);
		b = (m[0] == 0x0101);
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 1, true, false
