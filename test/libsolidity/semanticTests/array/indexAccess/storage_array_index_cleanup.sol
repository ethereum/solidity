contract C {
    uint256[256] values;

    function f() public returns (uint256) {
        uint8 index;
        assembly { index := 0x100 }
        values[0] = 0x1234;
        return values[index];
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x1234
