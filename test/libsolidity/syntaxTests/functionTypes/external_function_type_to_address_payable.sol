contract C {
    function f() public view returns (address payable) {
        return address(this.f);
    }
}
// ----
// TypeError: (85-100): Return argument type address is not implicitly convertible to expected type (type of first return variable) address payable.
