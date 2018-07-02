contract test {
    function fun(uint256 a) public returns (uint) {
        if (a >= 8) { return 2; } else { uint b = 7; }
    }
}
// ----
// Warning: (109-115): Unused local variable.
// Warning: (20-128): Function state mutability can be restricted to pure
