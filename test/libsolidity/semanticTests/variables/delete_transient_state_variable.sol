contract C {
    uint transient x;
    function f() public returns (uint) {
        x = 10;
        delete x;
        return x;
    }
}
// ====
// EVMVersion: >=cancun
// compileViaYul: false
// ----
// f() -> 0
