pragma abicoder               v2;
contract C {
    function f(uint256[][] calldata x) external { x[0]; }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// revertStrings: debug
// ----
// f(uint256[][]): 0x20, 1, 0x20, 2, 0x42 -> FAILURE, hex"08c379a0", 0x20, 23, "Calldata tail too short"
