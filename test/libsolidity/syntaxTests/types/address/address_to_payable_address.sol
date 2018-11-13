contract C {
    function f(address a) public pure {
        address b;
        address payable c = a;
        c = b;
    }
}
// ----
// TypeError: (80-101): Type address is not implicitly convertible to expected type address payable.
// TypeError: (115-116): Type address is not implicitly convertible to expected type address payable.
