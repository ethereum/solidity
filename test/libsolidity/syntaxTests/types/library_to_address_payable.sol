library L {
}
contract C {
    function f() public pure returns (address payable) {
        return address(L);
    }
}
// ----
// TypeError: (99-109): Return argument type address is not implicitly convertible to expected type (type of first return variable) address payable.
