library C {
    function f() private pure returns(uint[]) {}
}
// ----
// Warning: (50-56): Parameter is declared as memory. Use an explicit "memory" keyword to silence this warning.