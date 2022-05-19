contract test {
    function f() public pure returns (uint r) {
        uint32 x;
        uint8 y;
        assembly {
            x := 0xfffffffffe
            y := 0x102
        }
        unchecked { r = x**y; }
        return r;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 4
