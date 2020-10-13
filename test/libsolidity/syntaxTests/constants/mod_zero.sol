contract c {
    uint constant a1 = 0;
    uint constant a2 = 1;
    uint constant b1 = 3 % a1;
    uint constant b2 = 3 % (a2 - 1);
}
// ----
// TypeError 6020: (88-94): Operator % not compatible with types int_const 3 and int_const 0
