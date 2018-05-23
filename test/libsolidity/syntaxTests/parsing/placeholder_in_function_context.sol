contract c {
    function fun() returns (uint r) {
        var _ = 8;
        return _ + 1;
    }
}
// ----
// Warning: (59-64): Use of the "var" keyword is deprecated.
// Warning: (59-68): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (17-97): No visibility specified. Defaulting to "public". 
// Warning: (17-97): Function state mutability can be restricted to pure
