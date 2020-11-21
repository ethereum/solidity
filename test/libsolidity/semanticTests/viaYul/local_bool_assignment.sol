contract C {
    function f(bool a) public pure returns (bool x) {
        bool b = a;
        x = b;
    }
}
// ====
// compileViaYul: true
// compileToEwasm: also
// ----
// f(bool): true -> true
