contract test {
    function fun(uint256 a) {
        for (uint256 i = 0; i < 10; i++) {
            uint256 x = i; break; continue;
        }
    }
}
// ----
// Warning: (20-148): No visibility specified. Defaulting to "public". 
// Warning: (33-42): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (101-110): Unused local variable.
// Warning: (20-148): Function state mutability can be restricted to pure
