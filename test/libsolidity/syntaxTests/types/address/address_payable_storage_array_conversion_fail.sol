contract C {
    address payable[] a;
    address[] b;
    function f() public view {
        address payable[] storage c = a;
        address[] storage d = b;
        c = d;
    }
}
// ----
// TypeError: (172-173): Type address[] storage pointer is not implicitly convertible to expected type address payable[] storage pointer.
