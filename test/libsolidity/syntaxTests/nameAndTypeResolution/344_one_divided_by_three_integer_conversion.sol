contract test {
    function f() public {
        uint a = 1/3;
    }
}
// ----
// TypeError 9574: (50-62): Type rational_const 1 / 3 is not implicitly convertible to expected type uint256. Rational number is fractional, use an explicit conversion instead.
