contract test {
    function fun(uint256 a) returns (uint) {
        if (a >= 8) { return 2; } else { uint b = 7; }
    }
}
// ----
// Warning: (20-121): No visibility specified. Defaulting to "public". 
// Warning: (102-108): Unused local variable.
// Warning: (20-121): Function state mutability can be restricted to pure
