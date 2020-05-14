pragma experimental ABIEncoderV2;
contract C {
    function f(uint256[][] calldata x) external { x[0]; }
}
// ====
// EVMVersion: >=byzantium
// revertStrings: debug
// compileViaYul: also
// ----
// f(uint256[][]): 0x20, 1, 0x20, 2, 0x42 -> FAILURE, hex"08c379a0", 0x20, 23, "Calldata tail too short"
