contract A {
    modifier f() virtual { _; }
}
contract B {
    modifier f() virtual { _; }
}
contract C is A, B {
}
// ----
// TypeError 6480: (94-116): Derived contract must override modifier "f". Two or more base classes define modifier with same name.
