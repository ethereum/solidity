contract Test {
    function uintToBytes(uint256 h) public returns (bytes32 s) {
        return bytes32(h);
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// uintToBytes(uint256): left(0x616263) -> left(0x616263)
