contract A {
    modifier f() virtual { _; }
}
contract B {
    modifier f() virtual { _; }
}
contract C is A, B {
    modifier f() override(A,B) { _; }
}
// ----
