contract test {
    function run() public returns(int256 y) {
        return -int32(10) * -int64(20);
    }
}
// ====
// compileToEwasm: also
// ----
// run() -> 200
