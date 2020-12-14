contract C {
    function f() public {
        address payable a = address(new D());

        // This conversion makes no sense anyway.
        address payable b = address(D);
    }
}

contract D {
    receive() external payable {
    }
}
// ----
// TypeError 9574: (47-83): Type address is not implicitly convertible to expected type address payable.
// TypeError 9640: (164-174): Explicit type conversion not allowed from "type(contract D)" to "address".
// TypeError 9574: (144-174): Type address is not implicitly convertible to expected type address payable.
