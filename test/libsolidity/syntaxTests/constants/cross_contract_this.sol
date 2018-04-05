contract A {
    uint constant c = 0;
}
contract C {
    uint constant c = A.this.c;
}
// ----
// TypeError: (75-81): Member "this" not found or not visible after argument-dependent lookup in type(contract A)
