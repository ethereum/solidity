contract C {
    uint constant a = 1 / 0;
}
// ----
// TypeError 2271: (35-40): Built-in binary operator / cannot be applied to types int_const 1 and int_const 0.
