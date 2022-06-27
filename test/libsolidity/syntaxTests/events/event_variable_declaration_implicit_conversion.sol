contract C {
    event MyCustomEvent(uint);
    function f() public {
        bytes4 a = MyCustomEvent;
    }
}

// ----
// TypeError 9574: (78-102): Type event MyCustomEvent(uint256) is not implicitly convertible to expected type bytes4.
