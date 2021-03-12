contract C {
    function f(bool a) public pure returns (bool x) {
        bool b = a;
        x = b;
        assert(b);
    }
    function f2(bool a) public pure returns (bool x) {
        bool b = a;
        x = b;
        require(b);
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(bool): true -> true
// f(bool): false -> FAILURE, hex"4e487b71", 0x01
// f2(bool): true -> true
// f2(bool): false -> FAILURE
