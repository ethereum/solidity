contract C {
    uint constant b3 = 1 % 0;
}
// ----
// TypeError 2271: (36-41): Built-in binary operator % cannot be applied to types int_const 1 and int_const 0.
