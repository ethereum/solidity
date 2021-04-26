contract C {
	function dyn() public returns (bytes memory a, uint b, bytes20[] memory c, uint d) {
		a = "1234567890123456789012345678901234567890";
		b = type(uint).max;
		c = new bytes20[](4);
		c[0] = bytes20(uint160(1234));
		c[3] = bytes20(uint160(6789));
		d = 0x1234;
	}
	function f() public returns (bytes memory, uint, bytes20[] memory, uint) {
		return this.dyn();
	}
}
// ====
// EVMVersion: >homestead
// compileViaYul: also
// ----
// f() -> 0x80, -1, 0xe0, 0x1234, 40, "12345678901234567890123456789012", "34567890", 4, 97767552542602192590433234714624, 0, 0, 537879995309340587922569878831104
