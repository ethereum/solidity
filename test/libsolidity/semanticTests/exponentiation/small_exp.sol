contract test {
    function f() public pure returns (uint) {
        uint32 x;
        uint8 y;
        assembly {
            x := 0xfffffffffe
            y := 0x102
        }
        return x**y;
    }
}
// ----
// f() -> 4
