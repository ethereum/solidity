contract A { }
contract B is A {
    function f() public { B b = A(1); }
}
// ----
// TypeError: (59-69): Type contract A is not implicitly convertible to expected type contract B.
