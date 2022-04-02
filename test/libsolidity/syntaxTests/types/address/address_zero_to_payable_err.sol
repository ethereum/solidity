contract C {
    function f() public pure {
        address payable a = address(0);
    }
}
// ----
// TypeError 9574: (52-82='address payable a = address(0)'): Type address is not implicitly convertible to expected type address payable.
