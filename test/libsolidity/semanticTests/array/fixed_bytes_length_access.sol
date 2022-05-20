contract C {
    bytes1 a;

    function f(bytes32 x) public returns (uint256, uint256, uint256) {
        return (x.length, bytes16(uint128(2)).length, a.length + 7);
    }
}
// ====
// compileToEwasm: also
// ----
// f(bytes32): "789" -> 32, 16, 8
