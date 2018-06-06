contract test {
    function f() public {
        var c = 42 ** fixed(-1/4);
    }
}
// ----
// Warning: (50-55): Use of the "var" keyword is deprecated.
// TypeError: (58-75): Operator ** not compatible with types int_const 42 and fixed128x18
// Warning: (50-75): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
