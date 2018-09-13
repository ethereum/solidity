contract C {
    address payable[] a;
    address[] b;
    function f() public view {
        address payable[] storage c = a;
        address[] storage d = b;
        d = c; // TODO: this could be allowed in the future
    }
}
// ----
// TypeError: (172-173): Type address payable[] storage pointer is not implicitly convertible to expected type address[] storage pointer.
