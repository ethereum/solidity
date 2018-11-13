contract A {
    struct S {
        address payable a;
    }
    S s;
    function f() public {
        s.a = address(this);
    }
}
// ----
// TypeError: (110-123): Type address is not implicitly convertible to expected type address payable.
