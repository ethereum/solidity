contract test {
    function f() public returns(uint ret) {
        ret = 1;
        for (;;) {
            ret += 1;
            if (ret >= 10) break;
        }
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 10
