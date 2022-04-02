contract C {
    function f(address a) public pure {
        address b;
        address payable c = a;
        c = b;
    }
}
// ----
// TypeError 9574: (80-101='address payable c = a'): Type address is not implicitly convertible to expected type address payable.
// TypeError 7407: (115-116='b'): Type address is not implicitly convertible to expected type address payable.
