error MyCustomError(uint, bool);

contract C {
    function f() public {
        bytes4 a = MyCustomError;
    }
}

// ----
// TypeError 9574: (81-105): Type error MyCustomError(uint256,bool) is not implicitly convertible to expected type bytes4.
