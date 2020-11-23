pragma abicoder               v2;
contract C {
	function e(bytes memory a) public pure returns (uint) { return 7; }
}
// ====
// EVMVersion: >=byzantium
// revertStrings: debug
// compileViaYul: also
// ----
// e(bytes): 0x20, 7 -> FAILURE, hex"08c379a0", 0x20, 39, "ABI decoding: invalid byte array", " length"
