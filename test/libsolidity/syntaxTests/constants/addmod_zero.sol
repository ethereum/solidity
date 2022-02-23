contract c {
    uint constant a1 = 0;
    uint constant a2 = 1;
    uint constant b1 = addmod(3, 4, 0);
    uint constant b2 = addmod(3, 4, a1);
    uint constant b3 = addmod(3, 4, a2 - 1);
}
// ----
// TypeError 4195: (88-103): Arithmetic modulo zero.
// TypeError 4195: (128-144): Arithmetic modulo zero.
// TypeError 4195: (169-189): Arithmetic modulo zero.
