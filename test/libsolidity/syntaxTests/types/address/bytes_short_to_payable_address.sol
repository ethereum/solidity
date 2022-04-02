contract C {
    function f(bytes10 x) public pure returns (address payable) {
        return address(x);
    }
}
// ----
// TypeError 9640: (94-104='address(x)'): Explicit type conversion not allowed from "bytes10" to "address".
// TypeError 6359: (94-104='address(x)'): Return argument type address is not implicitly convertible to expected type (type of first return variable) address payable.
