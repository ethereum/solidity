contract test {
    function f() pure public {
        var i = -2;
        i;
    }
}
// ----
// Warning: (55-60): Use of the "var" keyword is deprecated.
// Warning: (55-65): The type of this variable was inferred as int8, which can hold values between -128 and 127. This is probably not desired. Use an explicit type to silence this warning.
