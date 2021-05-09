// This basically tests proper cleanup and conversion. It should not convert x to int8.
contract C {
    function f() public returns (int8) {
        uint8 x = 254;
        int8 y = 1;
        return y << x;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 0
