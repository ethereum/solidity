contract C {
    uint constant a = 1 / 0;
}
// ----
// TypeError 2271: (35-40): Binary operator / not compatible with types int_const 1 and int_const 0.
