contract test {
    function f() pure public returns (uint) {
        var i = 1;
        return i;
    }
}
// ----
// Warning: (70-75): Use of the "var" keyword is deprecated.
// Warning: (70-79): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
