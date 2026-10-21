contract C {
	function f() public pure returns (bytes32, bytes32, uint) {
		bytes32 a;
		bytes32 b;
		uint x;
		((a, b, ), x) = (("abc", "xyz", 0x2a), 0x4d);
		return (a, b, x);
	}
}
// ----
// f() -> 0x6162630000000000000000000000000000000000000000000000000000000000, 0x78797a0000000000000000000000000000000000000000000000000000000000, 0x4d
