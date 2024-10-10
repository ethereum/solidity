contract C {
    uint transient public x;

    function f() public {
        x = 8;
    }
    function g() public returns (uint) {
        x = 0;
        this.f();
        return x;
    }
    function h() public returns (uint) {
        return x;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// x() -> 0
// g() -> 8
// h() -> 0
