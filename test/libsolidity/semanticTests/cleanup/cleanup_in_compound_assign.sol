contract C {
    function test() public returns (uint256, uint256) {
        uint32 a = 0xffffffff;
        uint16 x = uint16(a);
        uint16 y = x;
        x /= 0x100;
        y = y / 0x100;
        return (x, y);
    }
}
// ====
// compileToEwasm: also
// ----
// test() -> 0xff, 0xff
