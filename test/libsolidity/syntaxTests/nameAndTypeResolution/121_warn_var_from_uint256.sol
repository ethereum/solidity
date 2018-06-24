contract test {
    function f() pure public {
        var i = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff;
        i;
    }
}
// ----
// Warning: (55-60): Use of the "var" keyword is deprecated.
// Warning: (55-129): The type of this variable was inferred as uint256, which can hold values between 0 and 115792089237316195423570985008687907853269984665640564039457584007913129639935. This is probably not desired. Use an explicit type to silence this warning.
