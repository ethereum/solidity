type MyInt is int256;
contract C {
    MyInt transient public x;

    function f() public {
        x = MyInt.wrap(2);
    }
    function g() public returns (MyInt) {
        x = MyInt.wrap(0);
        this.f();
        return x;
    }
    function h() public returns (MyInt) {
        return x;
    }
}
// ====
// EVMVersion: >=cancun
// compileViaYul: false
// ----
// x() -> 0
// g() -> 2
// h() -> 0
