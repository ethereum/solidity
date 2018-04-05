contract A {
    uint constant c = 0;
}
contract B is A {
}
contract C {
    uint constant c = B.super.c;
}
// ----
// TypeError: (95-102): Member "super" not found or not visible after argument-dependent lookup in type(contract B)
