contract c {
    uint constant a1 = 0;
    uint constant a2 = 1;
    uint constant b1 = 7 / a1;
    uint constant b2 = 7 / (a2 - 1);
}
// ----
// TypeError 6020: (88-94): Operator / not compatible with types int_const 7 and int_const 0
