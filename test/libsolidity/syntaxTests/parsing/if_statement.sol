contract test {
    function fun(uint256 a) returns (uint) {
        if (a >= 8) { return 2; } else { var b = 7; }
    }
}
// ----
// Warning: (102-107): Use of the "var" keyword is deprecated.
// Warning: (102-111): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (20-120): No visibility specified. Defaulting to "public". 
// Warning: (102-107): Unused local variable.
// Warning: (20-120): Function state mutability can be restricted to pure
