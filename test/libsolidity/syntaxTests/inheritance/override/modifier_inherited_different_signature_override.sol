contract A {
    modifier f(uint a) virtual { _; }
}
contract B {
    modifier f() virtual { _; }
}
contract C is A, B {
    modifier f() virtual override(A, B) { _; }
}
// ----
// TypeError 1078: (125-167): Override changes modifier signature.
