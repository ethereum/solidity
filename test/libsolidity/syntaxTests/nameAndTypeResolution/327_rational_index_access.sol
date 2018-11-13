contract test {
    function f() public {
        uint[] memory a;
        a[.5];
    }
}
// ----
// TypeError: (77-79): Type rational_const 1 / 2 is not implicitly convertible to expected type uint256. Try converting to type ufixed8x1 or use an explicit conversion.
