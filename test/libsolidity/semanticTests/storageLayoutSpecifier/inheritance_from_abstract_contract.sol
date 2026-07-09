abstract contract A {
    uint public x;
}

contract C is A layout at 42 {
    uint public y;
    function f() public returns (uint) {
        x = 12;
        x = x - 4;
        y = x + 2;
        return y;
    }
}
// ====
// targetContract: C
// ----
// f() -> 10
// x() -> 8
// y() -> 10
