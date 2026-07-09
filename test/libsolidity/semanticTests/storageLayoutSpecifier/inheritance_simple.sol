contract A {
    uint public x = 10;
}

contract C is A layout at 42 {
    uint public y;
    function f() public returns (uint) {
        x = x - 4;
        y = x + 2;
        return y;
    }
}
// ====
// targetContract: C
// ----
// f() -> 8
// x() -> 6
// y() -> 8
