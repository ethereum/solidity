interface I {
    function f(uint x) external returns (uint);
}

contract C is I layout at 42 {
    uint public y;
    function f(uint x) public returns (uint) {
        x = x - 4;
        y = x + 2;
        return y;
    }
}
// ====
// targetContract: C
// ----
// f(uint256): 8 -> 6
// y() -> 6
