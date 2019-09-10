contract C {
    function f(bool a) public pure returns (bool x) {
        bool b = a;
        x = b;
    }
}
// ====
// compileViaYul: true
// ----
// f(bool): true -> true
