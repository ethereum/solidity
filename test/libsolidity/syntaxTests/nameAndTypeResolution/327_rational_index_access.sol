contract test {
    function f() public {
        uint[] memory a;
        a[.5];
    }
}
// ----
// TypeError 7407: (77-79): Type rational_const 1 / 2 is not implicitly convertible to expected type uint256. Rational number is fractional, use an explicit conversion instead.
