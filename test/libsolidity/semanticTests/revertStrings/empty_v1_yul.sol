contract C {
	function f() public {
		revert("");
	}
	function g(string calldata msg) public {
		revert(msg);
	}
}
// ====
// ABIEncoderV1Only: true
// EVMVersion: >=byzantium
// compileViaYul: true
// revertStrings: debug
// ----
// f() -> FAILURE, hex"08c379a0", 0x20, 0
// g(string): "" -> FAILURE, hex"08c379a0", 0x20, 0
// g(string): 0x20, 0, "" -> FAILURE, hex"08c379a0", 0x20, 0
// g(string): 0x20, 0 -> FAILURE, hex"08c379a0", 0x20, 0
