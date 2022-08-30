error MyCustomError(uint, bool);
contract Test {
    function f() public {
        uint[] memory a;
        a[MyCustomError];
    }
}
// ----
// TypeError 7407: (110-123): Type error MyCustomError(uint256,bool) is not implicitly convertible to expected type uint256.
