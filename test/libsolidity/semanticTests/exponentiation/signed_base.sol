contract test {
    function f() public pure returns (int, int) {
        int32 x = -3;
        uint8 y1;
        uint8 y2;
        assembly {
            y1 := 0x102
            y2 := 0x103
        }
        return (x**y1, x**y2);
    }
}
// ----
// f() -> 9, -27
