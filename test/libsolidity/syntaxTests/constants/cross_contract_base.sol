contract A {
    uint constant c = 0;
}
contract B is A {
}
contract C {
    uint constant c = B.c;
}
// ----
// TypeError: (95-98): Member "c" not found or not visible after argument-dependent lookup in type(contract B)
