type MyInt is int;
contract C {
    function f() external pure returns (MyInt a) {
    }
    function g() external pure returns (MyInt b, MyInt c) {
        b = MyInt.wrap(int(1));
        c = MyInt.wrap(1);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0
// g() -> 1, 1
