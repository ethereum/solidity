contract C {
    function f() public view returns (address payable) {
        return this.f.address;
    }
}
// ----
// TypeError: (85-99): Return argument type address is not implicitly convertible to expected type (type of first return variable) address payable.
