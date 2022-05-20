contract C {
    function f(bool a) public pure returns (bool x) {
        bool b = a;
        x = b;
    }
}
// ====
// compileToEwasm: also
// ----
// f(bool): true -> true
