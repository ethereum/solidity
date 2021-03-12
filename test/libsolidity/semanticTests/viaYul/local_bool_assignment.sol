contract C {
    function f(bool a) public pure returns (bool x) {
        bool b = a;
        x = b;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(bool): true -> true
