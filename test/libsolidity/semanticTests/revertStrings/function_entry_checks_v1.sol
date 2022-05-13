pragma abicoder v1;
contract C {
    function t(uint) public pure {}
}
// ====
// EVMVersion: >=byzantium
// ABIEncoderV1Only: true
// revertStrings: debug
// compileViaYul: false
// ----
// t(uint256) -> FAILURE, hex"08c379a0", 0x20, 0x12, "Calldata too short"
