pragma abicoder v2;
contract C {
    function t(uint) public pure {}
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: false
// revertStrings: debug
// ----
// t(uint256) -> FAILURE, hex"08c379a0", 0x20, 34, "ABI decoding: tuple data too sho", "rt"
