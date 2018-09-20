contract C {
    function f(bytes32 x) public pure returns (address payable) {
        return address(x);
    }
}
// ----
// TypeError: (94-104): Explicit type conversion not allowed from "bytes32" to "address".
// TypeError: (94-104): Return argument type address is not implicitly convertible to expected type (type of first return variable) address payable.
