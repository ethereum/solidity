contract A {
    uint public x;
    function f() public returns (uint) {
        x = x + 1;
        return x;
    }
}

contract B is A {
    uint public y;
    function g() public virtual returns (uint) {
        y = y + 2;
        return y;
    }
}

contract C is B layout at 42 {
    uint public z;
    function test() public returns (uint){
        z = super.g() + A.f();
        return z;
    }
}
// ====
// targetContract: C
// ----
// f() -> 1
// g() -> 2
// test() -> 6
// x() -> 2
// y() -> 4
// z() -> 6
