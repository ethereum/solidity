contract A {
    uint public x;
    fallback () external {
        if (x == 2) return;
        x++;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// ()
// x() -> 1
// ()
// x() -> 2
// ()
// x() -> 2
// ()
// x() -> 2
