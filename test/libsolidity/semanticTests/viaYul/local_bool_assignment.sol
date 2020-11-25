contract C {
    function f(bool a) public pure returns (bool x) {
        bool b = a;
        x = b;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(bool): true -> true
