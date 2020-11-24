contract C {
    function f() public pure returns (uint) {
        uint8 b;
        assembly { b := 0xffff }
        return b;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 255
