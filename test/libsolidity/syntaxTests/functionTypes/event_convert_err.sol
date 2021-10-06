contract C {
    event That(uint a);
    function f() public {
        function (uint) pure g = That;
        emit That(2);
    }
}
// ----
// TypeError 9574: (71-100): Type event That(uint256) is not implicitly convertible to expected type function (uint256) pure. Special functions can not be converted to function types.
