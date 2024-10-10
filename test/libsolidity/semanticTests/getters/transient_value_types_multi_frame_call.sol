contract C {
    int8 public transient x;

    function f() public returns(int8) {
        x = -1;
        return this.h();
    }
    function g() public {
        x = x - 1;
    }
    function h() public returns(int8) {
        this.g();
        return this.x();
    }
}
// ====
// EVMVersion: >=cancun
// ----
// x() -> 0
// f() -> -2
// h() -> -1
// x() -> 0
