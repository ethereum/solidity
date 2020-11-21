contract test {
    uint8 x;
    uint v;
    function f() public returns (uint x, uint y, uint z) {
        uint16 a;
        uint32 b;
        assembly {
            a := 0x0f0f0f0f0f
            b := 0xff0fff0fff
        }
        x = a & b;
        y = a | b;
        z = a ^ b;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 3855, 268374015, 268370160
