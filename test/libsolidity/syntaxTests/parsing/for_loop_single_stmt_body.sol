contract test {
    function fun(uint256 a) {
        uint256 i = 0;
        for (i = 0; i < 10; i++)
            continue;
    }
}
// ----
// Warning: (20-129): No visibility specified. Defaulting to "public". 
// Warning: (33-42): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (20-129): Function state mutability can be restricted to pure
