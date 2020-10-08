contract test {
    function fun(uint256 a) public returns (uint) {
        if (a >= 8) { return 2; } else { uint b = 7; }
    }
}
// ----
// Warning 6321: (60-64): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 2072: (109-115): Unused local variable.
// Warning 2018: (20-128): Function state mutability can be restricted to pure
