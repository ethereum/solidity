contract C {
    function f(address a) public pure returns (address payable) {
        return address(address(a));
    }
}
// ----
// TypeError 6359: (94-113='address(address(a))'): Return argument type address is not implicitly convertible to expected type (type of first return variable) address payable.
