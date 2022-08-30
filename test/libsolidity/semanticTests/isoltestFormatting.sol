contract C {
    function f() public returns (uint[5] memory) {
        uint[5] memory a = [4, 11, 0x111, uint(3355443), 2222222222222222222];
        return a;
    }
    function g() public returns (uint[5] memory) {
        uint[5] memory a = [16, 256, 257, uint(0x333333), 0x1ed6eb565788e38e];
        return a;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 4, 11, 0x0111, 0x333333, 2222222222222222222
// g() -> 0x10, 0x0100, 0x0101, 0x333333, 2222222222222222222
