contract test {
    function fun(uint256 a) {
        uint256 x = 3 ** a;
    }
}
// ----
// Warning: (20-79): No visibility specified. Defaulting to "public". 
// Warning: (54-63): Unused local variable.
// Warning: (20-79): Function state mutability can be restricted to pure
