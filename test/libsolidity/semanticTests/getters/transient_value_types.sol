contract C {
    int8 public transient x;

    function f() public returns(int8) {
        x = -1;
        return this.x();
    }
}
// ====
// EVMVersion: >=cancun
// ----
// x() -> 0
// f() -> -1
// x() -> 0
