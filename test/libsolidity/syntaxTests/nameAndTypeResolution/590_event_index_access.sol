contract Test {
    event MyCustomEvent(uint);
    function f() public {
        uint[] memory a;
        a[MyCustomEvent];
    }
}
// ----
// TypeError 7407: (108-121): Type event MyCustomEvent(uint256) is not implicitly convertible to expected type uint256.
