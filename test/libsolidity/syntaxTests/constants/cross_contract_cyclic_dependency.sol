contract A {
    uint constant c = B.c;
}
contract B {
    uint constant c = A.c;
}
// ----
// TypeError: (17-38): The value of the constant c has a cyclic dependency via c.
// TypeError: (59-80): The value of the constant c has a cyclic dependency via c.
