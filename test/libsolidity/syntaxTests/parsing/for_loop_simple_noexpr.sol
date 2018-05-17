contract test {
        function fun(uint256 a) {
            uint256 i =0;
            for (;;) {
                uint256 x = i; break; continue;
            }
        }
    }
// ----
// Warning: (24-170): No visibility specified. Defaulting to "public". 
// Warning: (37-46): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (115-124): Unused local variable.
// Warning: (24-170): Function state mutability can be restricted to pure
