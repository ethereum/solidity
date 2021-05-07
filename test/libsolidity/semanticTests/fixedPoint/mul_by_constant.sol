contract c {
    function mul(fixed a) public pure returns (fixed c) {
        return a * 1.5;
    }
}
// ====
// compileViaYul: also
// ----
// mul(fixed128x18): 100 -> 0x96
