contract A {
    struct S { uint256 a; }
    enum E { V }
}
contract B {
    A.S x;
    A.E e;
}
contract C is A {
    A.S x;
    S y;
    A.E e;
    E f;
}
// ----
