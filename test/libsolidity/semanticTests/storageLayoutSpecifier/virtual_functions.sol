abstract contract A {
    uint public x;
    function f() public virtual returns (uint);
}

contract B is A {
    uint public y;
    function f() public override returns (uint) {
        x = 1;
        return x;
    }
    function g() public virtual returns (uint) {
        return y;
    }
}

contract C is B layout at 42 {
    uint public z;
    function g() public override returns (uint) {
        y = x + 2;
        return y;
    }
    function h() public returns (uint) {
        z = g() + 3;
        return z;
    }
}
// ====
// targetContract: C
// ----
// f() -> 1
// g() -> 3
// h() -> 6
// x() -> 1
// y() -> 3
// z() -> 6
