pragma experimental ABIEncoderV2;
contract C {
	function f(uint[] memory a) public pure returns (uint) { return 7; }
}
// ====
// EVMVersion: >=byzantium
// revertStrings: debug
// compileViaYul: also
// ----
// f(uint256[]): 0x20, 1 -> FAILURE, hex"08c379a0", 0x20, 43, "ABI decoding: invalid calldata a", "rray stride"
