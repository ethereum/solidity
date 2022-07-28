type Int is int16;

using {add as +} for Int;


function add(Int, Int) returns (Int) {
    B b = new B();
    return b.f();
}

contract B {
    Int s;
    function f() external returns (Int) {
        s = Int.wrap(3);
        return s;
    }
}

contract C {
    function test() public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}
// ----
// test() -> 3
// gas legacy: 119695
