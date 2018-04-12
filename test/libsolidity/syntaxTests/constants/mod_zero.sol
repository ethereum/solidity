contract c {
    uint constant a1 = 0;
    uint constant a2 = 1;
    uint constant b1 = 3 % a1;
    uint constant b2 = 3 % (a2 - 1);
}
// ----
// TypeError: (88-94): Modulo zero.
// TypeError: (119-131): Modulo zero.
