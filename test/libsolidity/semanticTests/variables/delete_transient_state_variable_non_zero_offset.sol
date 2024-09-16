contract C {
    uint128 transient x;
    uint128 transient y;

    function f() public returns (uint) {
        y = 10;
        delete y;
        return y;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 0
