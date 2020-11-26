contract C {
    function f() public pure returns (uint16 x) {
        uint8 y = uint8(0x78);
        x = y;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0x78
