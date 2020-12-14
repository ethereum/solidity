contract C {
    function f() public pure {
        address payable a = address(uint160(0));
        address payable b = address(bytes20(0));
        address payable c = address(this);
    }
}
// ----
// TypeError 9574: (52-91): Type address is not implicitly convertible to expected type address payable.
// TypeError 9574: (101-140): Type address is not implicitly convertible to expected type address payable.
// TypeError 9574: (150-183): Type address is not implicitly convertible to expected type address payable.
