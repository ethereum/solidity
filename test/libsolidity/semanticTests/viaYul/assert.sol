contract C {
    function f(bool a) public pure returns (bool x) {
        bool b = a;
        x = b;
        assert(b);
    }
    function fail() public pure returns (bool x) {
        x = true;
        assert(false);
    }
    function succeed() public pure returns (bool x) {
        x = true;
        assert(true);
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(bool): true -> true
// f(bool): false -> FAILURE, hex"4e487b71", 0x01
// fail() -> FAILURE, hex"4e487b71", 0x01
// succeed() -> true
