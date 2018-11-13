contract C {
    function f(address a) public pure returns (address payable) {
        return address(address(a));
    }
}
// ----
// TypeError: (94-113): Return argument type address is not implicitly convertible to expected type (type of first return variable) address payable.
