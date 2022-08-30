contract test {
    function f(bool cond) public returns (uint, uint) {
        return cond ? (1, 2) : (3, 4);
    }
}
// ====
// compileToEwasm: also
// ----
// f(bool): true -> 1, 2
// f(bool): false -> 3, 4
