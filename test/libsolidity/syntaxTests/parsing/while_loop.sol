contract test {
    function fun(uint256 a) {
        while (true) { uint256 x = 1; break; continue; } x = 9;
    }
}
// ----
// Warning: (20-115): No visibility specified. Defaulting to "public". 
// Warning: (33-42): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (20-115): Function state mutability can be restricted to pure
