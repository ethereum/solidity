contract A { }
contract B is A {
    function f() public { B b = A(address(1)); }
}
// ----
// TypeError 9574: (59-78): Type contract A is not implicitly convertible to expected type contract B.
